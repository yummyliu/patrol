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

class CHttpClient
{
	public:
		CHttpClient(void);
		~CHttpClient(void);

	public:
		int Get(const std::string & strUrl, std::string & strResponse);
		int GetJson(const std::string & strUrl, Json::Value & jsonResponse);
		void SetDebug(bool bDebug);

	private:
		bool m_bDebug;
};

CHttpClient::CHttpClient(void) :
	m_bDebug(false)
{

}

CHttpClient::~CHttpClient(void)
{

}


static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
	if(itype == CURLINFO_TEXT)
	{
		//printf("[TEXT]%s\n", pData);
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
 * @brief HTTP GET请求
 * @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
 * @param strResponse 输出参数,返回的内容
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

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	/**
	 * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	 * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	 */
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
	/**
	 * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	 * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	 */
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return res;
}
void CHttpClient::SetDebug(bool bDebug)
{
		m_bDebug = bDebug;
}


Json::Value getJson(const std::string & strUrl, const std::string & key)
{
	CHttpClient client;

	Json::Value jsonresp;
	auto status = client.GetJson(strUrl, jsonresp);
	if (status == CURLE_OK) {
		return jsonresp.get(key,"");
	}

	return 0;
}

