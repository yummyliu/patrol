/* =============================================================================
#     FileName: command.cpp
#         Desc: command function in commander write in cpp
#       Author: LiuYangming
#        Email: sdwhlym@126.com
#     HomePage: http://yummyliu.github.io
#      Version: 0.0.1
#   LastChange: 2018-08-01 15:28:45
#      History:
============================================================================= */

extern "C" {
#include <postgres.h>    //all C headers and macros go inside extern "C"
#include <utils/rel.h>
#include <curl/curl.h>

#include "command.h"


PG_FUNCTION_INFO_V1(getdbinfo);
}

#include <string>
#include <sstream>
#include <strstream>
using namespace std;


string RequestJSONValueAsync(string url, string key);


extern "C" {
	Datum getdbinfo(PG_FUNCTION_ARGS) {
		std::ostringstream oss;
		auto dbId = PG_GETARG_INT32(0);
		// std::string url("http://localhost:3000/dbinfo/"+std::to_string(dbId));
		auto url = "http://localhost:3000/dbs";
		auto ss = RequestJSONValueAsync(url,"dbinfo");
		PG_RETURN_CSTRING(ss.c_str());
	}
}
