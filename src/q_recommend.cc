/*************************************************************************
    @ File Name: q_recommend.cc
    @ Method:
    @ Author: Jerry Shi
    @ Mail: jerryshi0110@gmail.com 
    @ Created Time: 2015年08月20日 星期四 14时43分53秒
 ************************************************************************/
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <glog/logging.h>
#include "mongoose.h"
#include "recommendEngine.h"

//recommendengie
static std::string token_pth = "/work/mproj/workproj/kuaipan/recommendation/recommend/resource/dict";
static std::string dict_pth = "/work/mproj/workproj/kuaipan/recommendation/recommend/resource/";
recommendEngine gRecomm(token_pth,dict_pth);


void initGlog(const char* cProgram,const char* logDir)
{
	if(!boost::filesystem::exists(logDir))
	{
		boost::filesystem::create_directory(logDir);
	}

	char cInfoPath[100];
	char cErrPath[100];
	char cWarnPath[100];
	char cFatalPath[100];

	snprintf(cInfoPath,sizeof(cInfoPath),"%s%s",logDir,"/INFO_");
	snprintf(cErrPath,sizeof(cErrPath),"%s%s",logDir,"/ERROR_");
	snprintf(cWarnPath,sizeof(cWarnPath),"%s%s",logDir,"/WARNING_");
	snprintf(cFatalPath,sizeof(cFatalPath),"%s%s",logDir,"/FATAL_");

	google::InitGoogleLogging(cProgram);

	FLAGS_logbufsecs = 0; // no cache
	FLAGS_stop_logging_if_full_disk = true; // disk if full
	FLAGS_alsologtostderr = false; //close to stderr

	google::SetLogDestination(google::GLOG_INFO,cInfoPath);
	google::SetLogDestination(google::GLOG_ERROR,cErrPath);
	google::SetLogDestination(google::GLOG_WARNING,cWarnPath);
	google::SetLogDestination(google::GLOG_FATAL,cFatalPath);


}

void* sig_thread(void* arg)
{
    sigset_t *set = (sigset_t *)arg;
    int s, sig;
    for(;;)
    {
        s = sigwait(set, &sig);
        if (s != 0)
        {
            perror("wait signal failed.");
            exit(1);
        }
        LOG(INFO) << "got signal, ignore: " << sig << std::endl;
    }
    return 0;
}

void AddSigPipe()
{
    sigset_t maskset;
    pthread_t sig_thread_id;
    sigemptyset(&maskset);
    sigaddset(&maskset, SIGPIPE);
    int ret;
    ret = pthread_sigmask(SIG_BLOCK, &maskset, NULL);
    if (ret != 0)
    {
        perror("failed to block signal!");
        exit(1);
    }
    ret = pthread_create(&sig_thread_id, NULL, &sig_thread, (void*)&maskset);
    
    if (ret != 0)
    {
        perror("failed to create the singal handle thread.");
        exit(1);
    }
}


static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
	
    std::string rstr = "";
	std::string json_result = "";
	boost::posix_time::ptime time_start,time_end;
	boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse;

    switch (ev) {
    case MG_AUTH: return MG_TRUE;
    case MG_REQUEST:
    {
        bool succ = true;
        char buffer[65535];
		time_start = boost::posix_time::microsec_clock::universal_time();

        try {
				//get
                if(mg_get_var(conn, "query", buffer, sizeof(buffer))>0) {
                    std::string pvalue(buffer);
					std::cout << "test uri:" << pvalue << std::endl;
					LOG(INFO) << "Request URI:" << conn->uri;
                    boost::algorithm::trim(pvalue);
					rstr = pvalue;
                    //sleep(60.0);
                }
				//post
				std::string content(conn->content, conn->content_len);
				
				if(rstr.size() == 0)
					rstr = content;
            }
        catch(std::exception& ex) {
            std::cerr<<ex.what()<<std::endl;
            succ = false;
        }
		time_end = boost::posix_time::microsec_clock::universal_time();

		time_elapse = time_end - time_start;
		int request_time = time_elapse.ticks();

        if(succ && rstr.size() != 0) {
        gRecomm.jsonResults(rstr,json_result);
        }
		time_end = boost::posix_time::microsec_clock::universal_time();
		time_elapse = time_end - time_start;
		int total_time = time_elapse.ticks();

		if(rstr.size() ==0)
			json_result = "No query input!";
		else
			LOG(INFO) <<"Request time:" << request_time << "us\ttotal time:" 
				<< total_time <<"us\tQuery:" << rstr 
				<<"\tRecommend:" << json_result;

        //mg_printf_data(conn, "Hello! Requested URI is [%s] ", content.c_str());
        mg_printf_data(conn, json_result.c_str());
        return MG_TRUE;
    }
    default: return MG_FALSE;
    }
}

static void *serve(void *server) {
    for (;;) mg_poll_server((struct mg_server *) server, 1000);
    return NULL;
}

void start_http_server(int pool_size = 1) {
    std::vector<struct mg_server*> servers;
    for(int p=0;p<pool_size;p++) {
        std::string name = boost::lexical_cast<std::string>(p);
		LOG(INFO) << "Creat thread:" << name;
        struct mg_server *server;
        server = mg_create_server((void*)name.c_str(), ev_handler);
        if(p==0) {
            mg_set_option(server, "listening_port", "18663");
        }
        else {
            mg_copy_listeners(servers[0], server);
        }
        servers.push_back(server);
        //printf("Starting on port %s\n", mg_get_option(server, "listening_port"));
        //for (;;) {
        //    mg_poll_server(server, 1000);
        //}
        //mg_destroy_server(&server);
    }
    for(uint32_t p=0;p<servers.size();p++) {
        struct mg_server* server = servers[p];
        mg_start_thread(serve, server);
    }
    sleep(3600*24*365*10);
    //boost::mutex m_pause_mutex;
    //boost::condition_variable m_pause_changed;
    //boost::unique_lock<boost::mutex> lock(m_pause_mutex);
    //while(true)
    //{
    //    m_pause_changed.wait(lock);
    //}
}

int main(int argc, char** argv)
{
    //AddSigPipe();
	std::string logDir = "../log";
	initGlog(argv[0],logDir.c_str());

    int thread_num = 22;
	LOG(INFO) << "Start program...";
    //std::cerr<<"thread-num: "<<thread_num<<std::endl;
	while(1)
	{
		start_http_server(thread_num);
	}
    return EXIT_SUCCESS;
	
}



