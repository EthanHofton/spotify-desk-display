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
#include "tasks/spotify_task.h"
#include "tasks/wifi_task.h"
#include <memory>

static constexpr const char* TAG = "app_main";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Free internal: %u", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Free PSRAM:    %u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    gpio_install_isr_service(0);
    // init nvs
    NvsManager::get_instance();

    static std::shared_ptr<AppState> state = std::make_shared<AppState>();

    xTaskCreate(
        lvgl_task,
        "lvgl_task",
        8192,
        state.get(),
        5,
        NULL
    );

    xTaskCreate(
        wifi_task,
        "wifi_task",
        2048,
        state.get(),
        5,
        NULL
    );

    xTaskCreate(
        spotify_task,
        "spotify_task",
        16384,
        state.get(),
        5,
        NULL
    );
}
