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

#include <string>
#include <sstream>
#include <strstream>
#include <cpprest/http_client.h>


using namespace std;
using namespace utility;
using namespace web::http;
using namespace web;
using namespace web::http::client;
using namespace concurrency::streams;


ostringstream ss;

string RequestJSONValueAsync(string url, string key)
{

	http_client client(url);
	client.request(methods::GET)
		.then([](http_response response) -> pplx::task<json::value>
				{
				if(response.status_code() == status_codes::OK)
				{
				return response.extract_json();
				}

				// Handle error cases, for now return empty json value...
				return pplx::task_from_result(json::value());
				})
	.then([&key](pplx::task<json::value> previousTask){
			try
			{
			const json::value& v = previousTask.get();
			ss << v.as_object().at(key).serialize() <<endl;
			}
			catch (const http_exception& e)
			{
			// Print error.
			ss << e.what();
			}
			}).wait();

	return ss.str().c_str();
}
