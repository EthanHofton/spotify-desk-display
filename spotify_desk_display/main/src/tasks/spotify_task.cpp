#include "tasks/spotify_task.h"
#include "callback_events/wifi_events.h"
#include "esp_bit_defs.h"
#include "freertos/idf_additions.h"
#include "tasks/app_state.h"
#include "spotify_client/spotify_client.h"

static constexpr const char* TAG = "SpotifyTask";

void spotify_task(void* t_arg) {
    // AppState* app_state = GET_APP_STATE(t_arg);
    // EventGroupHandle_t evt_grp = xEventGroupCreate();
    //
    // // set the callback to wait for wifi setup
    // app_state->m_cb_manager.register_callback<WifiGotIp>([&evt_grp](const WifiGotIp& t_evt) {
    //     xEventGroupSetBits(evt_grp, BIT0);
    // });
    //
    // xEventGroupWaitBits(evt_grp, BIT0, pdFALSE, pdTRUE, portMAX_DELAY);
    //
    // SpotifyClient spotify_client;
    // std::optional<SpotifyPlayerState> player_state = spotify_client.get_player_state();
    //
    // if (player_state.has_value()) {
    //
    //     ESP_LOGI(TAG, "is_playing: %s", player_state.value().is_playing ? "true" : "false");
    //     ESP_LOGI(TAG, "device name: %s", player_state.value().device_name);
    //     ESP_LOGI(TAG, "volume: %d", player_state.value().volume_percent);
    //
    //     ESP_LOGI(TAG, "progress_ms: %d", player_state.value().progress_ms);
    //     
    //     if (std::holds_alternative<SpotifyPlayerState::TrackItem>(player_state.value().item)) {
    //         SpotifyPlayerState::TrackItem item = std::get<SpotifyPlayerState::TrackItem>(player_state.value().item);
    //
    //         ESP_LOGI(TAG, "album name: %s", item.album_name);
    //         ESP_LOGI(TAG, "artist name(s): %s", item.artist_name);
    //         ESP_LOGI(TAG, "track name: %s", item.name);
    //         ESP_LOGI(TAG, "image url: %s", item.album_art_url);
    //         ESP_LOGI(TAG, "duration: %d", item.duration_ms);
    //     }
    //
    //     ESP_LOGI(TAG, "context type: %d", player_state.value().context_type);
    //
    // }
    //
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
