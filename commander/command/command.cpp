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

#include "command.h"

	PG_FUNCTION_INFO_V1(getjson);
}

#include <string>
#include <sstream>
#include <strstream>

#include "httpclient.h"
using namespace std;

extern "C" {
	/* CREATE OR REPLACE FUNCTION getjson ()
	 *     RETURNS cstring
	 *     AS 'commander.so',
	 *     'getjson'
	 *     LANGUAGE C ;
	 */
	Datum getjson(PG_FUNCTION_ARGS) {
		std::ostringstream oss;
		auto dbId = PG_GETARG_INT32(0);

		// std::string url("http://localhost:3000/dbinfo/"+std::to_string(dbId));
		auto url = "http://localhost:3000/dbs";

		Json::Value ss = getJson(url,"dbinfo");

		try {
			Json::FastWriter fastWriter;
			std::string output = fastWriter.write(ss);
			elog(INFO, "msg: %s", output.c_str());

			HeapTuple rettuple;
			CStringGetDatum(output.c_str());
			PG_RETURN_CSTRING(pstrdup(output.c_str()));
		}catch(Json::LogicError e) {
			PG_RETURN_CSTRING(e.what());
		}
	}
}
