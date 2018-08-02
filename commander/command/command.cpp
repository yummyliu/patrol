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

#include <vector>
#include <string>
#include <sstream>

extern "C" {

	static std::string DownloadedResponse;

	static int writer(char *data, size_t size, size_t nmemb, std::string *buffer_in)
	{

	    // Is there anything in the buffer?
	    if (buffer_in != NULL)
	    {
	        // Append the data to the buffer
	        buffer_in->append(data, size * nmemb);

	        // How much did we write?
	        DownloadedResponse = *buffer_in;

	        return size * nmemb;
	    }

	    return 0;

	}

	std::string DownloadJSON(std::string URL)
	{
	    CURL *curl;
	    CURLcode res;
	    struct curl_slist *headers=NULL; // init to NULL is important
	    headers = curl_slist_append(headers, "Accept: application/json");
	    headers = curl_slist_append(headers, "Content-Type: application/json");
	    headers = curl_slist_append(headers, "charsets: utf-8");
	    curl = curl_easy_init();

	    if (curl)
	    {
			elog(LOG, "url-: %s", URL.c_str());
	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
	        curl_easy_setopt(curl, CURLOPT_HTTPGET,1);
	        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writer);
	        res = curl_easy_perform(curl);

	        if (CURLE_OK == res)
	        {
	            char *ct;
	            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
	            if((CURLE_OK == res) && ct)
	                return DownloadedResponse;
	        }
	    }

	    curl_slist_free_all(headers);
	}

    Datum
	getdbinfo(PG_FUNCTION_ARGS) {
	    std::ostringstream oss;

		auto dbId = PG_GETARG_INT32(0);
		std::string url("http://localhost:3000/dbinfo/"+std::to_string(dbId));

		std::string res = DownloadJSON(url);

		PG_RETURN_CSTRING(res.c_str());
	}
}
