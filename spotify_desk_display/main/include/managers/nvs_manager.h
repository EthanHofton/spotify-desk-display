#pragma once

#include "nvs.h"
#include <cstdint>
#include <string>
#include <map>
#include <optional>


enum NvsOpenMode {
    READONLY,
    READWRITE,
    READWRITE_PURGE
};

class NvsManager {
private:
    NvsManager();

public:

    static NvsManager& get_instance();
    ~NvsManager();
    
    std::string get_str(const char* t_namespace, const char *t_key);
    int32_t get_int(const char* t_namespace, const char *t_key);

private:

    std::map<std::string, nvs_handle_t> m_open_namespaces;
    bool is_init = false;

    std::optional<nvs_handle_t> open_ns_if_not_exist(const char* t_namespace);
};
