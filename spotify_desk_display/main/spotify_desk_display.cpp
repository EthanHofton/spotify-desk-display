#include "esp_log.h"
#include "managers/nvs_manager.h"
#include "display/lcd_display.h"

static const char* TAG = "app_main";

extern "C" void app_main(void) {
    LcdDisplay display = LcdDisplay();
    FrameBuffer fb = FrameBuffer(display.get_width(), display.get_height());
    fb.fill(Pixel::from_normalised(1, 0, 0));

    display.draw(fb);
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
