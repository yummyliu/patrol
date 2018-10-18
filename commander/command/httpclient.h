#ifndef __HTTPCLIENT_H
#define __HTTPCLIENT_H

#include <json/json.h>
#include <json/reader.h>

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

Json::Value getJson(const std::string & strUrl);
Json::Value getJsonByKey(const std::string & strUrl, const std::string & key);

#endif
