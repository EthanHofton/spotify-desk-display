#include "ui/base_page.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "misc/lv_timer.h"
#include <memory>

BasePage::BasePage(AppState* t_app_state) : m_app_state(t_app_state) {
    m_display = std::make_unique<LcdDisplay>(320, 240, 5);
    m_lvgl_manager = std::make_unique<LvglManager>(*m_display);
}
BasePage::~BasePage() {}

void BasePage::run() {
    init();

    while (1) {
        update();

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
