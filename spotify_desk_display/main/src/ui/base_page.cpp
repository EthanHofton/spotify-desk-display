#include "ui/base_page.h"

static constexpr const char* TAG = "BasePage";

BasePage::BasePage(AppState* t_app_state, PageManager* t_page_manager)
    : m_app_state(t_app_state), m_page_manager(t_page_manager) {}
BasePage::~BasePage() {
    for (auto token : m_callback_tokens) {
        ESP_LOGI(TAG, "Unregistering callback: %d", token);
        m_app_state->cb_manager->unregister(token);
    }
}

void BasePage::register_callback(CallbackToken t_token) {
    ESP_LOGI(TAG, "Registering callback: %d", t_token);
    m_callback_tokens.push_back(t_token);
}

void BasePage::on_button_press(const PageManagerQueueItem& t_qitem) {}
