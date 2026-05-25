#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include <any>
#include <functional>
#include <map>
#include <typeindex>

#define CALLBACK_CLASS_BIND(cb_manager, TEvent, func) cb_manager.register_callback<TEvent>(std::bind(&func, this, std::placeholders::_1))

class CallbackManager {
public:
    CallbackManager() {
        m_mutex = xSemaphoreCreateMutex();
    }
    
    template <typename TEvent>
    void register_callback(std::function<void(const TEvent&)> t_cb) {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_callbacks[typeid(TEvent)].push_back(t_cb);
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
            std::any_cast<std::function<void(const TEvent&)>>(cb)(event);
        }
    }

    ~CallbackManager() {
        vSemaphoreDelete(m_mutex);
    }
    
private:
    std::map<std::type_index, std::vector<std::any>> m_callbacks;
    SemaphoreHandle_t m_mutex;
};
