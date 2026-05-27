#pragma once

#include "http_client/http_response.h"
#include <map>
#include <string>

namespace Http {
    
    typedef std::map<std::string, std::string> Headers;

    HttpResponse get(const char* t_url, Headers t_headers);
    HttpResponse post(const char* t_url, Headers t_headers, const std::string& t_body);
}
