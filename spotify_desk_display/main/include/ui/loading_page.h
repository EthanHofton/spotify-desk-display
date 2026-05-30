#pragma once

#include "callback_events/wifi_events.h"
#include "misc/lv_types.h"
#include "ui/base_page.h"

class LoadingPage : public BasePage {
public:
    static constexpr const char* TAG = "LoadingPage";

    LoadingPage(AppState* t_app_state, PageManager* t_page_manager);
    virtual ~LoadingPage() override;

    virtual void init() override;
    virtual void update() override;
    virtual void on_button_press(const PageManagerQueueItem& t_qitem) override;

    void handle_wifi_event(const WifiEvent& t_event);
    void handle_wifi_got_ip(const WifiGotIp& t_event);

private:

    void set_loading_message(const char* message);       // Update label only, spinner untouched

    lv_obj_t* m_screen;
    lv_obj_t* m_status_label;   // Used in LOADING state
};
