#include "ui/spotify_player_page.h"
#include "core/lv_obj.h"
#include "widgets/label/lv_label.h"
#include "ui/page_manager.h"

SpotifyPlayerPage::SpotifyPlayerPage(AppState* t_app_state, PageManager* t_page_manager)
    : BasePage(t_app_state, t_page_manager) {}

SpotifyPlayerPage::~SpotifyPlayerPage() {}

void SpotifyPlayerPage::init() {
    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_screen_load(screen);

    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "Spotify Player Page - Hello, World!");
    lv_obj_center(label);
}

void SpotifyPlayerPage::update() {
}

void SpotifyPlayerPage::on_button_press(const PageManagerQueueItem& t_qitem) {
    ESP_LOGI(TAG, "Spotify Player Page - button pressed");
}
