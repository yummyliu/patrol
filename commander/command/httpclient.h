#ifndef __HTTPCLIENT_H
#define __HTTPCLIENT_H

#include <json/json.h>
#include <json/reader.h>

Json::Value getJson(const std::string & strUrl);
Json::Value getJsonByKey(const std::string & strUrl, const std::string & key);

#endif
