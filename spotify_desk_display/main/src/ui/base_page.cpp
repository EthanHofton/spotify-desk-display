#include "ui/base_page.h"

BasePage::BasePage(AppState* t_app_state, PageManager* t_page_manager)
    : m_app_state(t_app_state), m_page_manager(t_page_manager) {}
BasePage::~BasePage() {}

void BasePage::on_button_press(const PageManagerQueueItem& t_qitem) {}
