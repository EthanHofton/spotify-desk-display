#pragma once
#include "ui/base_page.h"
#include "lvgl.h"
#include <cstdint>

class WifiDashboard : public BasePage {
public:

    enum class ConnectionStatus : uint8_t {
        NoConnection = 0,
        Connecting,
        Connected
    };

    WifiDashboard(AppState* t_app_state, PageManager* t_page_manager);
    virtual ~WifiDashboard() override;
    virtual void init() override;
    virtual void update() override;
    virtual void on_button_press(const PageManagerQueueItem& t_qitem) override;

    void set_connection_status(ConnectionStatus t_status);
    void add_log_message(const char* t_msg);

private:
    // -- Layout
    lv_obj_t* m_screen        = nullptr;
    lv_obj_t* m_header        = nullptr;

    // -- Status panel
    lv_obj_t* m_status_panel  = nullptr;
    lv_obj_t* m_status_icon   = nullptr;
    lv_obj_t* m_status_label  = nullptr;

    // -- Log panel
    lv_obj_t* m_log_panel     = nullptr;
    lv_obj_t* m_log_header    = nullptr;
    lv_obj_t* m_log_textarea  = nullptr;

    // -- Footer
    lv_obj_t* m_footer        = nullptr;
    lv_obj_t* m_reconnect_btn = nullptr;
    lv_obj_t* m_reconnect_lbl = nullptr;

    // -- State
    ConnectionStatus m_status = ConnectionStatus::NoConnection;

    // -- Styles
    lv_style_t m_style_screen;
    lv_style_t m_style_header;
    lv_style_t m_style_panel;
    lv_style_t m_style_log;
    lv_style_t m_style_btn;
    lv_style_t m_style_btn_pressed;

    // -- Helpers
    void init_styles();
    void init_header();
    void init_status_panel();
    void init_log_panel();
    void init_footer();

    void apply_status_style();   // updates icon colour + label text from m_status

    static void on_reconnect_cb(lv_event_t* t_e);
};
