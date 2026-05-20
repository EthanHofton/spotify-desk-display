#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include <map>
#include <string>


class QueueManager {
public:
    QueueManager() = default;
    QueueManager(const QueueManager&) = delete;
    QueueManager& operator=(const QueueManager&) = delete;   
    ~QueueManager();

    template <typename T>
    void register_queue(const std::string& key, uint32_t length) {
        if (m_queues.contains(key)) {
            ESP_LOGW(TAG, "Queue with name %s allready registered", key);
            return;
        }

        m_queues[key] = xQueueCreate(length, sizeof(T));
    }
    
    template <typename T>
    bool post(const std::string& key, const T& item, TickType_t timeout = portMAX_DELAY) {
        if (!m_queues.contains(key)) {
            ESP_LOGW(TAG, "No Queue with name %s registered", key);
            return false;
        }

        xQueueSend(m_queues[key], &item, timeout);
        return true;
    }
    
    template <typename T>
    bool post_from_isr(QueueHandle_t t_queue_handle, const T& item) {
        BaseType_t higher_priority_woken = pdFALSE;
        xQueueSendFromISR(t_queue_handle, &item, &higher_priority_woken);
        portYIELD_FROM_ISR(higher_priority_woken);
        return true;
    }

    template <typename T>
    bool poll(const std::string& t_key, T& t_item, TickType_t t_timeout = 0) {
        if (!m_queues.contains(t_key)) {
            ESP_LOGW(TAG, "No Queue with name %s registered", t_key);
            return false;
        }
        return xQueueReceive(m_queues[t_key], &t_item, t_timeout) == pdTRUE;
    }

    QueueHandle_t get_queue_handle(const std::string& t_key);

private:

    static constexpr const char* TAG = "QueueManager";

    std::map<std::string, QueueHandle_t> m_queues;
};
