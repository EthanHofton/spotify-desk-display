#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include <any>
#include <functional>
#include <map>
#include <typeindex>
#include <algorithm>

#define CALLBACK_CLASS_BIND(cb_manager, TEvent, func) cb_manager->register_callback<TEvent>(std::bind(&func, this, std::placeholders::_1))
using CallbackToken = uint32_t;

class CallbackManager {
public:
    CallbackManager() {
        m_mutex = xSemaphoreCreateMutex();
    }
    
    template <typename TEvent>
    CallbackToken register_callback(std::function<void(const TEvent&)> t_cb) {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        CallbackToken token = ++m_next_token;
        m_callbacks[typeid(TEvent)].push_back({ token, t_cb });
        xSemaphoreGive(m_mutex);

        ESP_LOGI("test", "registering callback with token: %d, next token: %d", token, m_next_token);

        return token;
    }

    void unregister(CallbackToken t_token) {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        for (auto&[key, vec] : m_callbacks) {
            vec.erase(
                std::remove_if(vec.begin(), vec.end(), [t_token](const std::pair<CallbackToken, std::any>& p) { return p.first == t_token; }),
                vec.end()
            );
        }
        xSemaphoreGive(m_mutex);
    }

    template <typename TEvent>
    void fire(const TEvent& event) {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        auto it = m_callbacks.find(std::type_index(typeid(TEvent)));
        if (it == m_callbacks.end()) {
            xSemaphoreGive(m_mutex);
            return;
        }
        auto callbacks_copy = it->second;
        xSemaphoreGive(m_mutex);
        for (auto& cb : callbacks_copy) {
            std::any_cast<std::function<void(const TEvent&)>>(cb.second)(event);
        }
    }

    ~CallbackManager() {
        vSemaphoreDelete(m_mutex);
    }
    
private:
    std::map<std::type_index, std::vector<std::pair<CallbackToken, std::any>>> m_callbacks;
    SemaphoreHandle_t m_mutex;
    CallbackToken m_next_token = 0;
};

