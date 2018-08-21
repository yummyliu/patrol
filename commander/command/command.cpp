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

#include <postgres.h>
#include <utils/rel.h>
#include "command.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


PG_FUNCTION_INFO_V1(getdbinfo);

}

#include <string>
#include <sstream>
#include <strstream>

#include "httpclient.h"
using namespace std;

extern "C" {
	Datum getdbinfo(PG_FUNCTION_ARGS) {
		auto dbId = PG_GETARG_INT32(0);
		string command = "select pip, pport from patrol.dbinfo where id = "+to_string(dbId);
		elog(INFO, "%s",command.c_str());

		string pip;
		string pport;

		int ret=0;
		if ((ret = SPI_connect()) < 0)
			elog(INFO, "SPI_connect returned %d", ret);

		ret = SPI_exec(command.c_str(), 0);
		uint64 proc = SPI_processed;
		if (ret > 0 && SPI_tuptable != NULL)
		{
    	    TupleDesc tupdesc = SPI_tuptable->tupdesc;
    	    SPITupleTable *tuptable = SPI_tuptable;

    	    for (uint64 j = 0; j < proc; j++)
    	    {
    	        HeapTuple tuple = tuptable->vals[j];

				pip = SPI_getvalue(tuple, tupdesc, 1);
				pport = SPI_getvalue(tuple, tupdesc, 2);
    	    }
    	}
		SPI_finish();

		string url = "http://"+pip+":"+pport+"/dbinfo";
		Json::Value avs = getJson(url,"Top2diskSpace");

		elog(INFO, "url: %s", url.c_str());
		try {
			auto sb = Json::StreamWriterBuilder();
			Json::StreamWriter* writer = sb.newStreamWriter();
			ostringstream oss;
			writer->write(avs,&oss);
			elog(INFO, "msg: %s", oss.str().c_str());

			PG_RETURN_CSTRING(pstrdup(oss.str().c_str()));
		}catch(Json::LogicError e) {
			PG_RETURN_CSTRING(e.what());
		}
	}
}
