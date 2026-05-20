#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "managers/lvgl_manager.h"
#include "managers/nvs_manager.h"
#include "display/lcd_display.h"
#include "lvgl.h"

static const char* TAG = "app_main";

void framebuffer_test(LcdDisplay& display);
void nvs_manager_test();
void create_ui();

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Free internal: %u", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Free PSRAM:    %u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    static LcdDisplay display(320, 240, 5);  // static: outlives app_main's stack frame
    LvglManager manager = LvglManager(display);

    create_ui();

    while (1) {
        lv_timer_handler();         // process LVGL tasks (rendering, animations)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void create_ui() {
    lv_obj_t *scr = lv_scr_act();

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Hello World!");
    lv_obj_center(label);
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
}
