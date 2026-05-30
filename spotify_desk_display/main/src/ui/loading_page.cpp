#include "ui/loading_page.h"
#include "callback_events/wifi_events.h"
#include "core/lv_obj.h"
#include "managers/callback_manager.h"
#include "ui/colors.h"
#include "widgets/label/lv_label.h"
#include "ui/page_manager.h"
#include "widgets/spinner/lv_spinner.h"
#include "managers/wifi_manager.h"

LoadingPage::LoadingPage(AppState* t_app_state, PageManager* t_page_manager)
    : BasePage(t_app_state, t_page_manager) {}

LoadingPage::~LoadingPage() {}

void LoadingPage::init() {
    m_screen = lv_obj_create(nullptr);
    lv_screen_load(m_screen);

    lv_obj_set_style_bg_color(m_screen, SPOTIFY_BLACK, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(m_screen, LV_OPA_COVER, LV_PART_MAIN);
    //
    // ── Spinner ───────────────────────────────────────────────────────────
    lv_obj_t* spinner = lv_spinner_create(m_screen);
    lv_obj_set_size(spinner, 40, 40);
    lv_spinner_set_anim_params(spinner, 1500, 180); // 1 s per revolution, 90 deg arc
    lv_obj_set_style_arc_color(spinner, SPOTIFY_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, SPOTIFY_GREY,  LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 3, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 3, LV_PART_MAIN);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -16); // nudge up to make room for label
 
    // ── Message label ────────────────────────────────────────────────────
    m_status_label = lv_label_create(m_screen);
    lv_obj_set_style_text_color(m_status_label, SPOTIFY_GREY, LV_PART_MAIN);
    lv_obj_set_style_text_font(m_status_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(m_status_label, LV_ALIGN_CENTER, 0, 20); // below the spinner
 
    lv_label_set_text(m_status_label, "Loading...");

    register_callback(CALLBACK_CLASS_BIND(m_app_state->cb_manager, WifiEvent, LoadingPage::handle_wifi_event));
    register_callback(CALLBACK_CLASS_BIND(m_app_state->cb_manager, WifiGotIp, LoadingPage::handle_wifi_got_ip));
    
    // conenct wifi
    WifiManager::connect(m_app_state);
}

void LoadingPage::handle_wifi_event(const WifiEvent& t_event) {
    if (t_event == WifiEvent::CONNETING) {
        ESP_LOGI(TAG, "Wifi Event - Connecting");
    } else if (t_event == WifiEvent::FAILED) {
        ESP_LOGI(TAG, "Wifi Event - Failed");
    } else if (t_event == WifiEvent::DISCONNECTED) {
        ESP_LOGI(TAG, "Wifi Event - Disconnected");
    } else if (t_event == WifiEvent::CONNECTED) {
        ESP_LOGI(TAG, "Wifi Event - Connected");
    }
}

void LoadingPage::handle_wifi_got_ip(const WifiGotIp& t_event) {
    ESP_LOGI(TAG, "Got IP");
}

void LoadingPage::set_loading_message(const char* t_message) {
    lv_label_set_text(m_status_label, t_message);
}

void LoadingPage::update() {
}

void LoadingPage::on_button_press(const PageManagerQueueItem& t_qitem) {
    ESP_LOGI(TAG, "Loading page - button pressed");
}
