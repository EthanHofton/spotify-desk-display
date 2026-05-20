#include "ui/page_manager.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "queue/page_manager_queue_item.h"


PageManager::PageManager(AppState* t_state) {
    m_app_state = t_state;
    m_display = std::make_unique<LcdDisplay>(320, 240, 5);
    m_lvgl_manager = std::make_unique<LvglManager>(*m_display);
    m_app_state->m_queue_manager.register_queue<PageManagerQueueItem>(TAG, 5);
}

void PageManager::show_page(const std::string& t_key) {
    if (!m_registry.contains(t_key)) {
        ESP_LOGW(TAG, "Page %s does not exist", t_key.c_str());
    }

    // should delete old page
    m_current_page = m_registry[t_key](m_app_state, this);
    m_current_page->init();
}

void PageManager::run() {
    while (1) {
        // input poll
        PageManagerQueueItem qitem;
        if (m_app_state->m_queue_manager.poll(TAG, qitem, 0)) {
            if (m_current_page) {
                m_current_page->on_button_press(qitem);
            }
        }

        if (m_current_page) { m_current_page->update(); }

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void IRAM_ATTR page_manager_isr_handler(void* t_arg) {
    PageManagerButtonIsrArgs *args = static_cast<PageManagerButtonIsrArgs*>(t_arg);
    uint32_t now = xTaskGetTickCountFromISR();

    if ((now - args->last_press) < pdMS_TO_TICKS(200)) return;
    PageManagerQueueItem qitem = {
        .m_press_timestamp = now,
        .m_last_press_timestamp = args->last_press,
    };

    strncpy(qitem.m_key, args->key, sizeof(qitem.m_key) - 1);
    qitem.m_key[sizeof(qitem.m_key) - 1] = '\0';
    args->last_press = now;
    args->app_state->m_queue_manager.post_from_isr(args->queue_handle, qitem);
}

void PageManager::register_button(const std::string& t_button_key, gpio_num_t t_gpio_number) {
    PageManagerButtonIsrArgs args = {
        .app_state = m_app_state,
        .queue_handle = m_app_state->m_queue_manager.get_queue_handle(TAG)
    };
    strncpy(args.key, t_button_key.c_str(), sizeof(args.key) - 1);
    args.key[sizeof(args.key) - 1] = '\0';
    m_button_args.push_back(args);

    gpio_config_t config = {
        .pin_bit_mask = 1ULL << t_gpio_number,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&config);
    gpio_isr_handler_add(t_gpio_number, page_manager_isr_handler, &m_button_args.back());
}
