/* =============================================================================
#     FileName: httpclient.cpp
#         Desc:
#       Author: LiuYangming
#        Email: sdwhlym@126.com
#     HomePage: http://yummyliu.github.io
#      Version: 0.0.1
#   LastChange: 2018-08-07 11:03:09
#      History: https://blog.csdn.net/huyiyang2010/article/details/7664201
============================================================================= */
#include <string>
#include <iostream>

#include "httpclient.h"
#include "curl/curl.h"

using namespace std;

CHttpClient::CHttpClient(void) : m_bDebug(false)
{

}

CHttpClient::~CHttpClient(void)
{

}

static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
	if(itype == CURLINFO_TEXT)
	{
		printf("[TEXT]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if( NULL == str || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

static size_t OnWriteJson(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	Json::Value* jsonstr = dynamic_cast<Json::Value*>((Json::Value*)lpVoid);
	if( NULL == jsonstr || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	auto jsoncharreader = Json::CharReaderBuilder().newCharReader();
	jsoncharreader->parse(pData,pData+size*nmemb,jsonstr,NULL);

	return nmemb;
}
/**
 * @brief HTTP GET
 * @param strUrl
 * @param strResponse
 * @return 返回是否Post成功
 */
int CHttpClient::Get(const std::string & strUrl, std::string & strResponse)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	/**
	 * about option
	 * CURLOPT_NOSIGNAL:
	 * https://stackoverflow.com/questions/21887264/why-libcurl-needs-curlopt-nosignal-option-and-what-are-side-effects-when-it-is
	 * CURLOPT_CONNECTTIMEOUT:
	 *  In unix-like systems, this might cause signals to be used  unless  CURLOPT_NOSIGNAL(3) is set
	 */
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int CHttpClient::GetJson(const std::string & strUrl, Json::Value & jsonResponse)
{
	CURLcode res;
	/* returns a CURL easy handle
	 */
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteJson);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&jsonResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

	res = curl_easy_perform(curl);
	/* curl_easy_init call MUST have a corresponding call to
	 * curl_easy_cleanup(3) when the operation is complete.
	 */
	curl_easy_cleanup(curl);

	return res;
}

void CHttpClient::SetDebug(bool bDebug)
{
		m_bDebug = bDebug;
}

Json::Value getJson(const std::string & strUrl)
{
	CHttpClient client;

	Json::Value jsonresp;
	auto status = client.GetJson(strUrl, jsonresp);
	if (status == CURLE_OK) {
		return jsonresp;
	}

	return 0;
}

Json::Value getJsonByKey(const std::string & strUrl, const std::string & key)
{
	CHttpClient client;

	Json::Value jsonresp;
	auto status = client.GetJson(strUrl, jsonresp);
	if (status == CURLE_OK) {
		return jsonresp.get(key,"");
	}

	return 0;
}
