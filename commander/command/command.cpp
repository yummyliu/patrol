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
PG_FUNCTION_INFO_V1(getdbsinfo);
//PG_FUNCTION_INFO_V1(getdbtop);
//PG_FUNCTION_INFO_V1(launchfuncsnap);
//PG_FUNCTION_INFO_V1(launchtablesnap);
//PG_FUNCTION_INFO_V1(retcomposite);
}

#include <string>
#include <sstream>
#include <strstream>

#include "httpclient.h"

using namespace std;

string getPatrolUrl(string dbId) {
	string command = "select pip, pport from patrol.dbinfo where id = '"+dbId+"';";
	string pip;
	string pport;

	int ret=0;
	if ((ret = SPI_connect()) < 0) {
		elog(ERROR, "SPI_connect returned %d", ret);
	}

	ret = SPI_exec(command.c_str(), 0);
    elog(DEBUG1,"%s",command.c_str());
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

	return "http://"+pip+":"+pport;
}

vector<string> getPatrolUrls() {
	vector<string> rets;
	string command = "select pip, pport from patrol.dbinfo;";
	string pip;
	string pport;

	int ret=0;
	if ((ret = SPI_connect()) < 0) {
		elog(ERROR, "SPI_connect returned %d", ret);
	}

	ret = SPI_exec(command.c_str(), 0);
    elog(DEBUG1,"%s",command.c_str());
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
			rets.push_back("http://"+pip+":"+pport);
	    }
	}
	SPI_finish();

	return rets;
}
extern "C" {

	Datum getdbinfo(PG_FUNCTION_ARGS) {
	    FuncCallContext     *funcctx;
	    int                  call_cntr;
	    int                  max_calls;
	    TupleDesc            tupdesc;
	    AttInMetadata       *attinmeta;

	    if (SRF_IS_FIRSTCALL())
	    {
	        MemoryContext   oldcontext;
	        funcctx = SRF_FIRSTCALL_INIT();
	        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
	        /* total number of tuples to be returned */
	        funcctx->max_calls = 1;
	        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
	            ereport(ERROR,
	                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
	                     errmsg("function returning record called in context "
	                            "that cannot accept type record")));
	        attinmeta = TupleDescGetAttInMetadata(tupdesc);
	        funcctx->attinmeta = attinmeta;
	        MemoryContextSwitchTo(oldcontext);
	    }

		/* get metric data from remote patrol
		 * metric: dbinfo(default), db_top ...
		 */
		auto dbId = PG_GETARG_CSTRING(0);
		string metric = PG_GETARG_CSTRING(1);
		string url = getPatrolUrl(dbId)+metric;

		Json::Value avs = getJson(url);
		try {
			auto sb = Json::StreamWriterBuilder();
			Json::StreamWriter* writer = sb.newStreamWriter();
			ostringstream oss;
			writer->write(avs,&oss);
		}catch(Json::LogicError e) {
			elog(ERROR, "msg0: %s %s", url.c_str(), e.std::exception::what());
		}

	    /* stuff done on every call of the function */
	    funcctx = SRF_PERCALL_SETUP();
	    call_cntr = funcctx->call_cntr;
	    max_calls = funcctx->max_calls;
	    attinmeta = funcctx->attinmeta;

	    if (call_cntr < max_calls) /* do when there is more left to send */
	    {
	        char       **values;
	        HeapTuple    tuple;
	        Datum        result;

	        values = (char **) palloc(2 * sizeof(char *));
	        values[0] = (char *) palloc(64 * sizeof(char));
	        values[1] = (char *) palloc(10 * sizeof(char));

		    try {
	            snprintf(values[0], 16, "%d", avs["Maxage"].asInt());
	            snprintf(values[1], 16, "%f", avs["DiskUsage"].asFloat());
		    }catch(Json::LogicError e) {
		    	elog(ERROR, "msg2: %s %s ", url.c_str(), e.std::exception::what());
		    }

	        tuple = BuildTupleFromCStrings(attinmeta, values);
	        result = HeapTupleGetDatum(tuple);

	        /* clean up (this is not really necessary) */
	        pfree(values[0]);
	        pfree(values[1]);
	        pfree(values);

	        SRF_RETURN_NEXT(funcctx, result);
	    }
	    else    /* do when there is no more left */
	    {
	        SRF_RETURN_DONE(funcctx);
	    }
	}

	Datum getdbsinfo(PG_FUNCTION_ARGS) {
	    FuncCallContext     *funcctx;
	    int                  call_cntr;
	    int                  max_calls;
	    TupleDesc            tupdesc;
	    AttInMetadata       *attinmeta;

		static vector<string> urls;
	    if (SRF_IS_FIRSTCALL())
	    {
	        MemoryContext   oldcontext;
	        funcctx = SRF_FIRSTCALL_INIT();
	        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
	        /* total number of tuples to be returned */
			urls = getPatrolUrls();
	        funcctx->max_calls = urls.size();
	        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
	            ereport(ERROR,
	                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
	                     errmsg("function returning record called in context "
	                            "that cannot accept type record")));
	        attinmeta = TupleDescGetAttInMetadata(tupdesc);
	        funcctx->attinmeta = attinmeta;
	        MemoryContextSwitchTo(oldcontext);
	    }

		/* get metric data from remote patrol
		 * metric: dbinfo(default), db_top ...
		 */
		string metric = PG_GETARG_CSTRING(0);

	    /* stuff done on every call of the function */
	    funcctx = SRF_PERCALL_SETUP();
	    call_cntr = funcctx->call_cntr;
	    max_calls = funcctx->max_calls;
	    attinmeta = funcctx->attinmeta;

		elog(DEBUG1, "if %d %d", call_cntr, int(urls.size()));
		if (call_cntr < max_calls) /* do when there is more left to send */
	    {
			auto url = urls[call_cntr];
			elog(DEBUG1, "%s", url.c_str());
			Json::Value avs = getJson(url+metric);
			try {
				auto sb = Json::StreamWriterBuilder();
				Json::StreamWriter* writer = sb.newStreamWriter();
				ostringstream oss;
				writer->write(avs,&oss);
			}catch(Json::LogicError e) {
				elog(ERROR, "msg0: %s %s", url.c_str(), e.std::exception::what());
			}
	        char       **values;
	        HeapTuple    tuple;
	        Datum        result;

	        values = (char **) palloc(2 * sizeof(char *));
	        values[0] = (char *) palloc(64 * sizeof(char));
	        values[1] = (char *) palloc(10 * sizeof(char));

		    try {
	            snprintf(values[0], 16, "%d", avs["Maxage"].asInt());
	            snprintf(values[1], 16, "%f", avs["DiskUsage"].asFloat());
		    }catch(Json::LogicError e) {
		    	elog(ERROR, "msg2: %s %s ", url.c_str(), e.std::exception::what());
		    }

	        tuple = BuildTupleFromCStrings(attinmeta, values);
	        result = HeapTupleGetDatum(tuple);

	        /* clean up (this is not really necessary) */
	        pfree(values[0]);
	        pfree(values[1]);
	        pfree(values);

	        SRF_RETURN_NEXT(funcctx, result);
	    }
	    else    /* do when there is no more left */
	    {
			urls.erase(urls.begin());
	        SRF_RETURN_DONE(funcctx);
	    }
	}
}
