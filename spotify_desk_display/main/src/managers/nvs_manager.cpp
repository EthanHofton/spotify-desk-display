#include "managers/nvs_manager.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#define INIT_GAURD(ret_val) if (this->is_init == false) { ESP_LOGI(TAG, "Init falied, returning"); return ret_val;}

static const char* TAG = "NvsManager";

NvsManager& NvsManager::get_instance() {
    static NvsManager instance;
    return instance;
}

NvsManager::NvsManager() {
    this->is_init = false;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "No free pages or new nvs version found, earasing and trying again");
        nvs_flash_erase();
        ret = nvs_flash_init();
    }

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Initalised NVS");
        this->is_init = true;
    } else {
        ESP_LOGI(TAG, "Failed to Initalise NVS");
    }
}

std::string NvsManager::get_str(const char* t_namespace, const char* t_key) {
    INIT_GAURD("");

    std::optional<nvs_handle_t> handle = open_ns_if_not_exist(t_namespace);
    if (!handle.has_value()) {
        return "";
    }

    size_t required_size = 0;
    esp_err_t err = nvs_get_str(handle.value(), t_key, nullptr, &required_size);
    if (err != ESP_OK) {
        return "";
    }

    std::string value(required_size, '\0');
    err = nvs_get_str(handle.value(), t_key, value.data(), &required_size);
    if (err != ESP_OK) {
        return {};
    }

    if (!value.empty() && value.back() == '\0') {
        value.pop_back();
    }

    return value;
}

int32_t NvsManager::get_int(const char* t_namespace, const char* t_key) {
    INIT_GAURD(0);
    std::optional<nvs_handle_t> handle = open_ns_if_not_exist(t_namespace);
    if (!handle.has_value()) {
        return 0;
    }
    int32_t value = 0;
    esp_err_t err = nvs_get_i32(handle.value(), t_key, &value);
    if (err != ESP_OK) {
        return 0;
    }
    return value;
}

std::optional<nvs_handle_t> NvsManager::open_ns_if_not_exist(const char* t_namespace) {
    if (this->m_open_namespaces.find(t_namespace) == this->m_open_namespaces.end()) {
        ESP_LOGI(TAG, "No namespace open found in existing handles, opening new ns for %s", t_namespace);

        nvs_handle_t handle;
        if (nvs_open(t_namespace, NVS_READONLY, &handle) != ESP_OK) {
            ESP_LOGE(TAG, "NVS open failed for %s", t_namespace);
            return {};
        }

        this->m_open_namespaces[std::string(t_namespace)] = handle;
        return handle;
    }

    return this->m_open_namespaces[std::string(t_namespace)];
}

NvsManager::~NvsManager() {
    for (auto& handle_pair : this->m_open_namespaces) {
        ESP_LOGI(TAG, "Closing NVS namespace: %s", handle_pair.first.c_str());
        nvs_close(handle_pair.second);
    }
}
