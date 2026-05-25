#pragma once

#include "esp_netif_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "esp_event.h"
#include "tasks/app_state.h"

class WifiManager {
public:
    static constexpr const char* TAG = "WifiManager";

    WifiManager(AppState* t_state);
    ~WifiManager();

    void connect();
    void disconnect();

    void deinit();

private:

    static void event_handler(void* t_arg, esp_event_base_t t_event_base, int32_t t_event_id, void* t_event_data);

private:

    static bool s_wifi_init;

    AppState* m_app_state;
    EventGroupHandle_t m_event_group;
    esp_event_handler_instance_t m_instance_any_id;
    esp_event_handler_instance_t m_instance_got_ip;
    int m_retry_count = 0;
    int m_max_retries = 3;
    bool m_wifi_connected = false;
    esp_netif_t* m_netif_handle;
};
