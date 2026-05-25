#include "display/lcd_display.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "managers/nvs_manager.h"
#include "tasks/app_state.h"
#include "tasks/lvgl_task.h"
#include "tasks/wifi_task.h"

static const char* TAG = "app_main";

void framebuffer_test(LcdDisplay& display);
void nvs_manager_test();

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Free internal: %u", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Free PSRAM:    %u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    gpio_install_isr_service(0);
    // init nvs
    NvsManager::get_instance();

    static AppState state;

    xTaskCreate(
        lvgl_task,
        "lvgl_task",
        8192,
        &state,
        5,
        NULL
    );

    xTaskCreate(
        wifi_task,
        "wifi_task",
        2048,
        &state,
        5,
        NULL
    );

    nvs_manager_test();
}

void framebuffer_test(LcdDisplay& display) {
    FrameBuffer fb = FrameBuffer(display.get_width(), display.get_height());
    fb.fill(Pixel::from_normalised(1,1,1));

    for (int y = 0; y < display.get_height(); y++) {
        for (int x = 0; x < display.get_width(); x++) {
            fb.set_pixel(Point(x, y), Pixel::from_normalised(
                (float)x / display.get_width(),
                0,
                (float)y / display.get_height()
            ));
        }
    }

    display.draw_framebuffer(fb);
}

void nvs_manager_test() {
    NvsManager nvs_manager = NvsManager::get_instance();

    std::string client_id = nvs_manager.get_str("spotify_creds", "client_id");
    ESP_LOGI(TAG, "Loaded spotify client id from nvs with NvsManager: %s", client_id.c_str());

    std::string client_secret = nvs_manager.get_str("spotify_creds", "client_secret");
    ESP_LOGI(TAG, "Loaded spotify client secret from nvs with NvsManager: %s", client_secret.c_str());

    std::string refresh_token = nvs_manager.get_str("spotify_creds", "refresh_token");
    ESP_LOGI(TAG, "Loaded spotify refresh token from nvs with NvsManager: %s", refresh_token.c_str());

    std::string token_expire = nvs_manager.get_str("spotify_creds", "token_expire");
    ESP_LOGI(TAG, "Loaded spotify token expire from nvs with NvsManager: %s", token_expire.c_str());

    std::string wifi_ssid = nvs_manager.get_str("wifi_creds", "ssid");
    ESP_LOGI(TAG, "Loaded wifi ssid from nvs with NvsManager: %s", wifi_ssid.c_str());

    std::string wifi_password = nvs_manager.get_str("wifi_creds", "password");
    ESP_LOGI(TAG, "Loaded wifi password from nvs with NvsManager: %s", wifi_password.c_str());
}
