#include "queue/queue_manager.h"

QueueHandle_t QueueManager::get_queue_handle(const std::string& t_key) {
    if (!m_queues.contains(t_key)) {
        ESP_LOGE(TAG, "Trying to get queue handle that wasnt registed: %s", t_key.c_str());
        abort();
    }

    return m_queues[t_key];
}

QueueManager::~QueueManager() {
    for (auto& [key, handle] : m_queues) {
        vQueueDelete(handle);
    }
}
