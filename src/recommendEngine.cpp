/*************************************************************************
    @ File Name: recommendEngine.cpp
    @ Method: recommend engine,query recommendation,correction,etc.
    @ Author: Jerry Shi
    @ Mail: jerryshi0110@gmail.com 
    @ Created Time: 2015年07月17日 星期五 15时52分57秒
 ************************************************************************/
#include "recommendEngine.h"

bool Is_subset(vector<std::size_t> v1,vector<std::size_t> v2);
void caculateNM(vector<std::size_t> v1,vector<std::size_t> v2,std::size_t& cnt);

static std::string timestamp = ".RecommendEngineTimestamp";

//construct,and initialize
recommendEngine::recommendEngine(const std::string& token_dir,const std::string& workdir)
	:token_dir_(token_dir)
	,workdir_(workdir)
	,timestamp_(0)
	,indexer_(NULL)
	,isNeedIndex(false)
{
	//initial dictionary and resoure directory
	std::string dict = workdir_ + "dict";
	if(!boost::filesystem::exists(dict + "/dict/"))
	{
		boost::filesystem::create_directory(dict + "/dict/");
	}

	//get timestamp
	std::string resource = workdir_;
    resource += "/";
	resource += "resource";
	if(!boost::filesystem::exists(resource))
	{
		//std::cerr << "No original query sources directory!\n";
	}
	else
	{
		resource += timestamp;
		std::ifstream in;
		in.open(resource.c_str(),std::ifstream::in);
		if(in)
			in >> timestamp_;
		in.close();
	}

	indexer_ = new indexEngine(token_dir_,dict + "/dict/"); //set token path and dictionary path
	isNeedIndex = indexer_->open();//load dictionary to memory
}

recommendEngine::~recommendEngine()
{
	if(NULL != indexer_)
	{
		delete indexer_;
		indexer_ = NULL;
	}
}

void recommendEngine::getCandicate(const std::string& userQuery,Terms2QidMap& terms2qIDs,
                    QueryIdataMap& queryIdata,QueryCateMap& query2Cate,String2IntMap& termsIdMap)
{
	//buildEngine();
	if(0 == userQuery.length())
		return;
   termsIdMap = indexer_->search(userQuery,terms2qIDs,queryIdata,query2Cate);
   //std::cout <<"tetst--terms2qIDs size:" << terms2qIDs_.size() << "\t queryIdata size:" 
	//   << queryIdata_.size()<< std::endl;
   //inputQuery = "";
   //jsonResult.clear();
   //inputQuery = userQuery;
}

//no results recommendation
void recommendEngine::recommendNoResults(Terms2QidMap& terms2qIDs,QueryIdataMap& queryIdata
                                        ,QueryCateMap& query2Cateults,Json::Value& jsonResult
                                        ,String2IntMap& termsIdMap,std::string inputQuery)
{
	//no candicate or no terms
	if(0 == terms2qIDs.size() || 0 == queryIdata.size() || 0 == termsIdMap.size())
		return;
//	std::cout << "tersm id size:" << terms2qIDs_.size() << "\tqueryIdata size:" << queryIdata_.size()
//			<< "\t input query terms size:" << termsIdMap.size() << std::endl;
	vector<std::size_t> qTermsID;
	vector<std::size_t> termsID;
	String2IntMapIter termsIter;
	qTermsID.clear();

	//find biggest score
	Terms2QidMapIter termsIdIter;
	bool subset = false;
	bool Not_subset = false;
	float bigScore1 = 0.0;
	float bigScore2 = 0.0;

	std::string res1 = "";
	std::string res2 = "";
	std::string big_term = "";

	for(termsIter = termsIdMap.begin(); termsIter != termsIdMap.end(); ++termsIter)
	{
		if(big_term.length() < termsIter->first.length())
			big_term = termsIter->first;
		termsID.push_back(termsIter->second);
	}

	for(termsIdIter = terms2qIDs.begin(); termsIdIter != terms2qIDs.end(); ++termsIdIter)
	{
		//std::cout << "test1-" << termsIdIter->first <<"term query vector size:" << 
		//	termsIdIter->second.size()<<std::endl;
		for(std::size_t i = 0; i < termsIdIter->second.size(); ++i)
		{
			qTermsID = queryIdata[termsIdIter->second[i]].tid;
			//std::cout << "qTermsID size:" << qTermsID.size() << std::endl;
			float weight = (float) queryIdata[termsIdIter->second[i]].counts / (
			queryIdata[termsIdIter->second[i]].hits + (float)0.3*queryIdata[termsIdIter->second[i]].counts);
			if(Is_subset(termsID,qTermsID))
			{
				subset = true;

				float score = (float) weight * qTermsID.size();
				if(bigScore1 < score)
				{
					bigScore1 = score;  //score
					res1 = queryIdata[termsIdIter->second[i]].text; // query
					//std::cout << "\tsubset score:" << bigScore1 << "\t query:" << res1 << std::endl;
				}
			}
			else
			{
				Not_subset = true;
				//caculateNM(termsID,qTermsID,cnt);
				float tscore = (float) weight * qTermsID.size();
				if(bigScore2 < tscore)
				{
					bigScore2 = tscore;
					res2 = queryIdata[termsIdIter->second[i]].text; //get query 
					//std::cout << "Not subset score:" << bigScore2 << "\t query:" << res2 << std::endl;
				}
			}
		}
	}
	
	//get the most similarity query
	std::string ss = "";
	float b_score = 0.0;
	if(subset)
	{
		ss = res1;
		b_score = bigScore1;
	}
	else if(Not_subset)
	{
		ss = res2;
		b_score = bigScore2;
	}
	//check the suggestion
	if(ss.length() <= 3 && b_score !=0)
		ss = big_term;
//	std::cout << "input query:" << inputQuery << std::endl;
	if(inputQuery == ss)
		ss = "";
    
	jsonResult["NoResult_Recommend"] = ss;
//	std::cout << "the most similar query is :" << ss << "\t biggest score:" << b_score << std::endl;
}

//query correction
void recommendEngine::recommendCorrection()
{
}

//related query recommendation
void recommendEngine::recommend(Terms2QidMap& terms2qIDs,QueryIdataMap& queryIdata
                        ,QueryCateMap& query2Cate,Json::Value& jsonResult,String2IntMap& termsIdMap
                        ,std::string inputQuery,const std::size_t TopK)
{
	//check
	if(0 == terms2qIDs.size() || 0 == queryIdata.size() || 0 == termsIdMap.size())
		return;

	vector<std::size_t> qTermsID;
	vector<std::size_t> termsID;
	map<std::string,float> queryScoreMap;


	qTermsID.clear();
	termsID.clear();
	queryScoreMap.clear();

	Terms2QidMapIter termsIdIter;
	String2IntMapIter termsIter;
	float queryScore = 0.0;
	std::size_t cnt = 0;
	std::string queryText = "";

	for(termsIter = termsIdMap.begin(); termsIter != termsIdMap.end(); ++termsIter)
	{
		termsID.push_back(termsIter->second);
	}
	float weight = 0.0;
	float similar = 0.0;
	//caculate score
	for(termsIdIter = terms2qIDs.begin(); termsIdIter != terms2qIDs.end(); ++termsIdIter)
	{
		for(std::size_t j = 0; j < termsIdIter->second.size(); ++j)
		{
			qTermsID = queryIdata[termsIdIter->second[j]].tid;
			caculateNM(termsID,qTermsID,cnt);

			similar = (float) cnt / (qTermsID.size() + 0.1) ;
			weight = (float)log(queryIdata[termsIdIter->second[j]].hits + 2.0)/(qTermsID.size() +0.1);
			queryScore = (float) weight * similar;
			queryText = queryIdata[termsIdIter->second[j]].text;
			Normalize::normalize(queryText);
			if(queryScoreMap.end() == queryScoreMap.find(queryText))
				queryScoreMap.insert(make_pair(queryText,queryScore));
		//	std::cout << "query:" << queryText <<"\tcontain NM:" << cnt
		//		<<"\tscore:" << queryScore << std::endl;
		}
	}

	//get TopK 
    vector<PAIR> queryScoreVector(queryScoreMap.begin(),queryScoreMap.end());
	sort(queryScoreVector.begin(),queryScoreVector.end(),cmpByValue());
	std::size_t upperBound;
	Json::Value recommend;
	if(TopK < queryScoreVector.size())
		upperBound = TopK;
	else
		upperBound = queryScoreVector.size();
	for(std::size_t i = 0; i < upperBound; ++i)
	{
		if(3 >= queryScoreVector[i].first.length() || inputQuery == queryScoreVector[i].first
				|| jsonResult["NoResult_Recommend"] == queryScoreVector[i].first)
			continue;
		recommend.append(queryScoreVector[i].first);
	}
	jsonResult["recommedndation"] = recommend;
}

//need add new datas
bool recommendEngine::isNeedAdd()
{
	std::string path = workdir_ + "resource/";
	path += "newdata.txt";
	std::size_t mt = boost::filesystem::last_write_time(path);
	if(mt > timestamp_)
		return true;
	else
		return false;	
}

//build index engine
bool recommendEngine::isNeedBuild()
{
	//check time
	std::time_t mt;
	if(0 != timestamp_)
	{
		mt = time(NULL) - timestamp_;
//		std::cout << "times invertal:" << mt << std::endl;
	if(isNeedIndex || mt > 100)
		return true;
	else
	{
//		std::cout << "do not need indexing!\n";
		return false;
	}
	}
	else
		return true;
}

//final recommendation results
void recommendEngine::jsonResults(const std::string& userQuery,std::string& res)
{
    Json::Value jsonResult;
    Terms2QidMap terms2qIDs;
    QueryIdataMap queryIdata;
    QueryCateMap query2Cate;
    String2IntMap termsIdMap;

	getCandicate(userQuery,terms2qIDs,queryIdata,query2Cate,termsIdMap);

	recommendNoResults(terms2qIDs,queryIdata,query2Cate,jsonResult,termsIdMap,userQuery);
	recommend(terms2qIDs,queryIdata,query2Cate,jsonResult,termsIdMap,userQuery);

    Json::Value catJson;
    Json::Value category;
    vector<string> cate;

	QueryCateMapIter cateIter;
	for(cateIter = query2Cate.begin();cateIter != query2Cate.end(); ++cateIter)
    {
        category.clear();
        for(std::size_t i = 0; i < cateIter->second.size();++i)
        {
            cate.clear();
            catJson.clear();
            boost::split(cate,cateIter->second[i],boost::is_any_of(">"));
            for(std::size_t j = 0; j < cate.size()-1; ++j)
                catJson.append(cate[j]);
            category.append(catJson);
        }
    }
    jsonResult["category"] = category;
	res = jsonResult.toStyledString();
}

void recommendEngine::clear()
{
	//clear dictornary in directory
	std::string dict_dir = workdir_ + "dict/dict/";
	if(boost::filesystem::exists(dict_dir))
	{
		boost::filesystem::remove_all(dict_dir);
	}
	boost::filesystem::create_directory(dict_dir);

}

void recommendEngine::buildEngine()
{
	std::string Tpth = workdir_ + "resource/";
	Tpth += timestamp;
	ofstream out;

	if(isNeedBuild())
		
	{
		std::cout << "Build the whole dictonary!\n";
		std::string pth = workdir_ + "resource/query.txt";
		indexer_->clear();
		indexer_->indexing(pth);
		clear();
		indexer_->flush();
		isNeedIndex = false;
		
		//update timestamp
		timestamp_ = time(NULL);
		out.open(Tpth.c_str(),std::ofstream::out | std::ofstream::trunc);
		out << timestamp_;
		out.close();
	}
	 if(isNeedAdd())
	{
		std::cout << "Add new datas into dictionary!\n";
		std::string path = workdir_ + "resource/newdata.txt";
		//indexer_->indexing(path);
		//indexer_->flush();
		isNeedIndex = false;

		//update timestamp
		timestamp_ = time(NULL);
		out.open(Tpth.c_str(),std::ofstream::out | std::ofstream::trunc);
		out << timestamp_;
		out.close();
	}
}


bool Is_subset(vector<std::size_t> v1,vector<std::size_t> v2)
{
	if(0 == v1.size() || 0 == v2.size())
		return false;
	boost::unordered_map<std::size_t,std::size_t> sets;
	sets.clear();
	
	for(std::size_t i = 0; i < v1.size(); ++i)
	{
		sets.insert(make_pair(v1[i],1));
	}
	
	for(std::size_t j = 0; j < v2.size(); ++j)
	{
		if(sets.end() == sets.find(v2[j]))
			return false;
	}
	return true;
}

void caculateNM(vector<std::size_t> v1,vector<std::size_t> v2,std::size_t& cnt)
{
	if(0 == v1.size() || 0 == v2.size())
	{
		cnt = 0;
		return;
	}
	boost::unordered_map<std::size_t,std::size_t> sets;
	sets.clear();
	cnt = 0;
	for(std::size_t i = 0; i < v1.size(); ++i)
		sets.insert(make_pair(v1[i],1));
	for(std::size_t j = 0; j < v2.size(); ++j)
	{
		if(sets.end() != sets.find(v2[j]))
			cnt += 1;
	}
}
