#pragma once

#include "display/lcd_display.h"
#include "managers/lvgl_manager.h"
#include "tasks/app_state.h"
#include "ui/base_page.h"
#include <functional>
#include <list>
#include <memory>
#include <map>

struct PageManagerButtonIsrArgs {
    char key[64];
    AppState* app_state;
    QueueHandle_t queue_handle;
    uint32_t last_press = 0;
};

void page_manager_isr_handler(void* t_arg);

class PageManager {
public:
    PageManager(AppState* t_state);
    
    // use type traits to ensure T is of type BasePage
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<BasePage, T>>>
    void register_page(const std::string& t_key) {
        if (m_registry.contains(t_key)) {
            ESP_LOGW(TAG, "Page %s allready exists", t_key.c_str());
        }

        m_registry[t_key] = [](AppState* state, PageManager* page_manager) {
            return std::make_unique<T>(state, page_manager);
        };
    }
    
    void show_page(const std::string& t_key);
    void run();

    void register_button(const std::string& t_button_key, gpio_num_t t_gpio_num);

    inline LcdDisplay& get_display() { return *m_display; }
    
private:
    static constexpr const char* TAG = "PageManager";

    AppState* m_app_state;
    std::unique_ptr<LcdDisplay> m_display;
    std::unique_ptr<LvglManager> m_lvgl_manager;
    
    std::unique_ptr<BasePage> m_current_page;
    std::map<std::string, std::function<std::unique_ptr<BasePage>(AppState*, PageManager*)>> m_registry;
    std::list<PageManagerButtonIsrArgs> m_button_args;
};
