#pragma once

#include <cstdint>
#include <string>

struct HttpResponse {
    uint16_t status_code;
    std::string body;
    bool is_error = false;
};
