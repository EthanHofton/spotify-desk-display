#include "managers/wifi_manager.h"
#include "callback_events/wifi_events.h"
#include "freertos/idf_additions.h"
#include "managers/nvs_manager.h"
#include "queue/wifi_command.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

bool WifiManager::s_wifi_init = false;

WifiManager::WifiManager(AppState* t_state) : m_app_state(t_state) {
    if (!s_wifi_init) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        m_netif_handle = esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        m_app_state->m_queue_manager.register_queue<WifiCommand>(TAG, 5);

        s_wifi_init = true;
    }
}

void WifiManager::connect() {
    if (m_wifi_connected) {
        ESP_LOGW(TAG, "Wifi allready connected");
        return;
    }

    if (m_event_group != nullptr) {
        vEventGroupDelete(m_event_group);
    }
    m_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler,
        (void*)this,
        &m_instance_any_id
    ));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        (void*)this,
        &m_instance_got_ip
    ));

    NvsManager nvs_manager = NvsManager::get_instance();
    std::string wifi_ssid = nvs_manager.get_str("wifi_creds", "ssid");
    std::string wifi_password = nvs_manager.get_str("wifi_creds", "password");

    wifi_config_t wifi_config = {};
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    strncpy((char*)wifi_config.sta.ssid, wifi_ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, wifi_password.c_str(), sizeof(wifi_config.sta.password) - 1);


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());


    EventBits_t bits = xEventGroupWaitBits(
        m_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to SSID: %s with password %s", wifi_ssid.c_str(), wifi_password.c_str());
        m_wifi_connected = true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s with password %s", wifi_ssid.c_str(), wifi_password.c_str());
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

void WifiManager::deinit() {
    if (!s_wifi_init) {
        return;
    }

    // Disconnect first (unregisters handlers, clears state)
    disconnect();

    // Stop the Wi-Fi driver
    ESP_ERROR_CHECK(esp_wifi_stop());

    // Deinit the driver, freeing its memory
    ESP_ERROR_CHECK(esp_wifi_deinit());

    // Destroy the netif for STA mode and the default event loop
    esp_netif_destroy_default_wifi(m_netif_handle);

    // Mark stack as uninitialized so the constructor can reinit if needed
    s_wifi_init = false;

    // Clean up the event group
    if (m_event_group != nullptr) {
        vEventGroupDelete(m_event_group);
        m_event_group = nullptr;
    }

    ESP_LOGI(TAG, "WiFi deinitialized");
}

void WifiManager::event_handler(void* t_arg, esp_event_base_t t_event_base, int32_t t_event_id, void* t_event_data) {
    WifiManager* self = static_cast<WifiManager*>(t_arg);

    if (t_event_base == WIFI_EVENT && t_event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(self->TAG, "STA START");
        self->m_app_state->m_cb_manager.fire<WifiEvent>(WifiEvent::CONNETING);
        esp_wifi_connect();
    } else if (t_event_base == WIFI_EVENT && t_event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (self->m_retry_count < self->m_max_retries) {
            self->m_app_state->m_cb_manager.fire<WifiEvent>(WifiEvent::DISCONNECTED);
            esp_wifi_connect();
            self->m_retry_count++;
            ESP_LOGI(self->TAG, "Retring to connect to the AP");
        } else {
            self->m_app_state->m_cb_manager.fire<WifiEvent>(WifiEvent::FAILED);
            xEventGroupSetBits(self->m_event_group, WIFI_FAIL_BIT);
        }
    } else if (t_event_base == IP_EVENT && t_event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(t_event_data);
        self->m_app_state->m_cb_manager.fire<WifiGotIp>({ .ip = event->ip_info.ip });
        ESP_LOGI(self->TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        self->m_retry_count = 0;
        xEventGroupSetBits(self->m_event_group, WIFI_CONNECTED_BIT);
    } else if (t_event_base == WIFI_EVENT && t_event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(self->TAG, "STA Connected");
        self->m_app_state->m_cb_manager.fire<WifiEvent>(WifiEvent::CONNECTED);
    }
}

void WifiManager::disconnect() {
    esp_err_t err = esp_wifi_disconnect();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_disconnect failed: %s", esp_err_to_name(err));
    }


    // Unregister event handlers so no reconnect logic fires
    if (m_instance_any_id != nullptr) {
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
            WIFI_EVENT, ESP_EVENT_ANY_ID, m_instance_any_id));
        m_instance_any_id = nullptr;
    }

    if (m_instance_got_ip != nullptr) {
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
            IP_EVENT, IP_EVENT_STA_GOT_IP, m_instance_got_ip));
        m_instance_got_ip = nullptr;
    }

    m_retry_count = 0;

    // Clear both bits so connect() can block cleanly if called again
    xEventGroupClearBits(m_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    m_wifi_connected = false;

    ESP_LOGI(TAG, "Disconnected from WiFi");
}
