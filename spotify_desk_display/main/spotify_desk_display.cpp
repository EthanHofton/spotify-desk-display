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

static const char* TAG = "app_main";

void framebuffer_test(LcdDisplay& display);

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

    xTaskCreate(
        spotify_task,
        "spotify_task",
        16384,
        &state,
        5,
        NULL
    );
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
