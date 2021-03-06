/*************************************************************************
    @ File Name: recommendEngine.h
    @ Method:
    @ Author: Jerry Shi
    @ Mail: jerryshi0110@gmail.com 
    @ Created Time: 2015年07月17日 星期五 15时52分32秒
 ************************************************************************/
#ifndef RECOMMENDENGINE_H
#define RECOMMENDENGINE_H

#include <iostream>
#include <map>

#include "json/json.h"
#include "indexEngine.h"
#include "normalize.h"
#include "time.h"
#include "math.h"

typedef pair<std::string,float> PAIR;

// sort  ascending
struct cmpByValue
{
	bool operator()(const PAIR& lhs, const PAIR& rhs)
	{
		return lhs.second > rhs.second;
	}
};

class recommendEngine
{
	public:
		recommendEngine(const std::string& token_dir,const std::string& dict_pth);
		~recommendEngine();

	public:
			void getCandicate(const std::string& userQuery,Terms2QidMap& terms2qIDs,
			  QueryIdataMap& queryIdata,QueryCateMap& query2Cate,String2IntMap& termsIdMap); //get candicate
			void recommendNoResults(Terms2QidMap& terms2qIDs,QueryIdataMap& queryIdata
			                        ,QueryCateMap& query2Cate,Json::Value& jsonResult,String2IntMap& termsIdMap,std::string inputQuery); //
			void recommendCorrection();
			void recommend(Terms2QidMap& terms2qIDs,QueryIdataMap& queryIdata,
			              QueryCateMap& query2Cate,Json::Value& jsonResult,String2IntMap& termsIdMap,
			              std::string inputQuery,const std::size_t TopK = 9);
			bool isNeedBuild();
			bool isNeedAdd();
			void jsonResults(const std::string& userQuery,std::string& res);
			void buildEngine();

	private:
			std::string token_dir_;
			std::string workdir_;
			std::size_t timestamp_;

			indexEngine* indexer_;
			//Terms2QidMap terms2qIDs_;
			//QueryIdataMap queryIdata_;
            //QueryCateMap query2Cate_;

			bool isNeedIndex;
			//String2IntMap termsIdMap; //terms ,it's hash value
			//Json::Value jsonResult;
			//std::string inputQuery;

			void clear();
};



#endif //recommendEngine.h
