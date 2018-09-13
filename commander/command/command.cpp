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

PG_FUNCTION_INFO_V1(getdbtop);
PG_FUNCTION_INFO_V1(getdbinfo);
PG_FUNCTION_INFO_V1(launchfuncsnap);
PG_FUNCTION_INFO_V1(launchtablesnap);
PG_FUNCTION_INFO_V1(retcomposite);
}

#include <string>
#include <sstream>
#include <strstream>

#include "httpclient.h"

using namespace std;

string getPatrolUrl(int dbId) {
	string command = "select pip, pport from patrol.dbinfo where id = "+to_string(dbId);
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

	string url = "http://"+pip+":"+pport;

	return url;
}

extern "C" {

	Datum
	retcomposite(PG_FUNCTION_ARGS)
	{
	    FuncCallContext     *funcctx;
	    int                  call_cntr;
	    int                  max_calls;
	    TupleDesc            tupdesc;
	    AttInMetadata       *attinmeta;

	    /* stuff done only on the first call of the function */
	    if (SRF_IS_FIRSTCALL())
	    {
	        MemoryContext   oldcontext;

	        /* create a function context for cross-call persistence */
	        funcctx = SRF_FIRSTCALL_INIT();

	        /* switch to memory context appropriate for multiple function calls */
	        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

	        /* total number of tuples to be returned */
	        funcctx->max_calls = PG_GETARG_UINT32(0);

	        /* Build a tuple descriptor for our result type */
	        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
	            ereport(ERROR,
	                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
	                     errmsg("function returning record called in context "
	                            "that cannot accept type record")));

	        /*
	         * generate attribute metadata needed later to produce tuples from raw
	         * C strings
	         */
	        attinmeta = TupleDescGetAttInMetadata(tupdesc);
	        funcctx->attinmeta = attinmeta;

	        MemoryContextSwitchTo(oldcontext);
	    }

	    /* stuff done on every call of the function */
	    funcctx = SRF_PERCALL_SETUP();

	    call_cntr = funcctx->call_cntr;
	    max_calls = funcctx->max_calls;
	    attinmeta = funcctx->attinmeta;

	    if (call_cntr < max_calls)    /* do when there is more left to send */
	    {
	        char       **values;
	        HeapTuple    tuple;
	        Datum        result;

	        /*
	         * Prepare a values array for building the returned tuple.
	         * This should be an array of C strings which will
	         * be processed later by the type input functions.
	         */
	        values = (char **) palloc(3 * sizeof(char *));
	        values[0] = (char *) palloc(16 * sizeof(char));
	        values[1] = (char *) palloc(16 * sizeof(char));
	        values[2] = (char *) palloc(16 * sizeof(char));

	        snprintf(values[0], 16, "%d", 1 * PG_GETARG_INT32(1));
	        snprintf(values[1], 16, "%d", 2 * PG_GETARG_INT32(1));
	        snprintf(values[2], 16, "%d", 3 * PG_GETARG_INT32(1));

	        /* build a tuple */
	        tuple = BuildTupleFromCStrings(attinmeta, values);

	        /* make the tuple into a datum */
	        result = HeapTupleGetDatum(tuple);

	        /* clean up (this is not really necessary) */
	        pfree(values[0]);
	        pfree(values[1]);
	        pfree(values[2]);
	        pfree(values);

	        SRF_RETURN_NEXT(funcctx, result);
	    }
	    else    /* do when there is no more left */
	    {
	        SRF_RETURN_DONE(funcctx);
	    }
	}

	Datum getdbinfo(PG_FUNCTION_ARGS) {
	    FuncCallContext     *funcctx;
	    int                  call_cntr;
	    int                  max_calls;
	    TupleDesc            tupdesc;
	    AttInMetadata       *attinmeta;

	    /* stuff done only on the first call of the function */
	    if (SRF_IS_FIRSTCALL())
	    {
	        MemoryContext   oldcontext;

	        /* create a function context for cross-call persistence */
	        funcctx = SRF_FIRSTCALL_INIT();

	        /* switch to memory context appropriate for multiple function calls */
	        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

	        /* total number of tuples to be returned */
	        funcctx->max_calls = 1;

	        /* Build a tuple descriptor for our result type */
	        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
	            ereport(ERROR,
	                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
	                     errmsg("function returning record called in context "
	                            "that cannot accept type record")));

	        /*
	         * generate attribute metadata needed later to produce tuples from raw
	         * C strings
	         */
	        attinmeta = TupleDescGetAttInMetadata(tupdesc);
	        funcctx->attinmeta = attinmeta;

	        MemoryContextSwitchTo(oldcontext);
	    }
		/* get metric data from remote patrol
		 * metric: dbinfo(default), db_top ...
		 */
		auto dbId = PG_GETARG_INT32(0);
		string metric = PG_GETARG_CSTRING(1);
		string url = getPatrolUrl(dbId)+metric;

		Json::Value avs = getJson(url);
		try {
			auto sb = Json::StreamWriterBuilder();
			Json::StreamWriter* writer = sb.newStreamWriter();
			ostringstream oss;
			writer->write(avs,&oss);
		}catch(Json::LogicError e) {
			elog(ERROR, "msg1: %s", e.std::exception::what());
		}

	    /* stuff done on every call of the function */
	    funcctx = SRF_PERCALL_SETUP();

	    call_cntr = funcctx->call_cntr;
	    max_calls = funcctx->max_calls;
	    attinmeta = funcctx->attinmeta;

	    if (call_cntr < max_calls)    /* do when there is more left to send */
	    {
	        char       **values;
	        HeapTuple    tuple;
	        Datum        result;

	        /*
	         * Prepare a values array for building the returned tuple.
	         * This should be an array of C strings which will
	         * be processed later by the type input functions.
	         */
	        values = (char **) palloc(2 * sizeof(char *));
	        values[0] = (char *) palloc(64 * sizeof(char));
	        values[1] = (char *) palloc(10 * sizeof(char));

	        //snprintf(values[0], 16, "%d", 1 * PG_GETARG_INT64(1));
	        snprintf(values[0], 16, "%d", avs["Maxage"].asInt());
	        //snprintf(values[1], 16, "%f", 2 * PG_GETARG_INT32(1));
	        snprintf(values[1], 16, "%f", avs["DiskUsage"].asFloat());

	        /* build a tuple */
	        tuple = BuildTupleFromCStrings(attinmeta, values);

	        /* make the tuple into a datum */
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
	Datum getdbtop(PG_FUNCTION_ARGS) {
		auto dbId = PG_GETARG_INT32(0);
		string metric = PG_GETARG_CSTRING(1);

		string url = getPatrolUrl(dbId)+"/dbtop";
		Json::Value avs = getJson(url);
		elog(INFO, "get json from url: %s", url.c_str());

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

	/* TODO
	 */
	Datum launchfuncsnap(PG_FUNCTION_ARGS) {
		auto dbId = PG_GETARG_INT32(0);
		string metric = PG_GETARG_CSTRING(1);

		string url = getPatrolUrl(dbId)+"/dbtop";
		Json::Value avs = getJson(url);
		elog(INFO, "get json from url: %s", url.c_str());

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

	/* TODO
	 */
	Datum launchtablesnap(PG_FUNCTION_ARGS) {
		auto dbId = PG_GETARG_INT32(0);
		string metric = PG_GETARG_CSTRING(1);

		string url = getPatrolUrl(dbId)+"/dbtop";
		Json::Value avs = getJson(url);
		elog(INFO, "get json from url: %s", url.c_str());

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
