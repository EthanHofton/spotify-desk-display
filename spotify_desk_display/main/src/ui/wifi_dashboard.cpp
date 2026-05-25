#include "ui/wifi_dashboard.h"
#include "display/lv_display.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "ui/page_manager.h"
#include <cstring>
#include <cstdio>
#include <format>
#include "managers/wifi_manager.h"
#include "queue/wifi_command.h"

// ─────────────────────────────────────────────
//  Colour palette
// ─────────────────────────────────────────────
static const lv_color_t CLR_BG         = lv_color_hex(0x0D1117);
static const lv_color_t CLR_PANEL      = lv_color_hex(0x161B22);
static const lv_color_t CLR_BORDER     = lv_color_hex(0x30363D);
static const lv_color_t CLR_ACCENT     = lv_color_hex(0x238636);
static const lv_color_t CLR_WARN       = lv_color_hex(0xD29922);
static const lv_color_t CLR_ERR        = lv_color_hex(0xDA3633);
static const lv_color_t CLR_TEXT       = lv_color_hex(0xE6EDF3);
static const lv_color_t CLR_TEXT_MUTED = lv_color_hex(0x8B949E);
static const lv_color_t CLR_BTN        = lv_color_hex(0x238636);
static const lv_color_t CLR_BTN_PRESS  = lv_color_hex(0x2EA043);

// ─────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────
WifiDashboard::WifiDashboard(AppState* t_app_state, PageManager* t_page_manager)
    : BasePage(t_app_state, t_page_manager) {}

WifiDashboard::~WifiDashboard() {
    // LVGL cleans up child objects when the screen is deleted
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
    }
}

void WifiDashboard::update() {
    // poll queues
    LogMsgQueueItem log_qitem;
    if (m_app_state->m_queue_manager.poll<LogMsgQueueItem>("wifi_dashboard_log", log_qitem)) {
        this->add_log_message(log_qitem.msg);
    }

    SetState state_qitem;
    if (m_app_state->m_queue_manager.poll<SetState>("wifi_dashboard_state", state_qitem)) {
        this->set_connection_status(state_qitem.state);
    }
}

void WifiDashboard::on_button_press(const PageManagerQueueItem& t_qitem) {
    this->add_log_message(std::format("Button Pressed at {} with key {}", t_qitem.m_press_timestamp, t_qitem.m_key).c_str());
    // this->m_page_manager->show_page("hello_world");
    this->m_app_state->m_queue_manager.post(WifiManager::TAG, WifiCommand::CONNECT);
}

void WifiDashboard::wifi_event_handler(const WifiEvent& t_event) {
    ESP_LOGI(WifiDashboard::TAG, "Wifi Event from WifiDashboard");

    if (t_event == WifiEvent::CONNECTED) {
        m_app_state->m_queue_manager.post<SetState>("wifi_dashboard_state", { .state = WifiDashboard::ConnectionStatus::Connected });
        m_app_state->m_queue_manager.post<LogMsgQueueItem>("wifi_dashboard_log", { .msg = "Connected to wifi" } );
    } else if (t_event == WifiEvent::DISCONNECTED || t_event == WifiEvent::FAILED) {
        m_app_state->m_queue_manager.post<SetState>("wifi_dashboard_state", { .state = WifiDashboard::ConnectionStatus::NoConnection });
        m_app_state->m_queue_manager.post<LogMsgQueueItem>("wifi_dashboard_log", { .msg = "Failed to connect..." } );
    } else if (t_event == WifiEvent::CONNETING) {
        m_app_state->m_queue_manager.post<SetState>("wifi_dashboard_state", { .state = WifiDashboard::ConnectionStatus::Connecting });
        m_app_state->m_queue_manager.post<LogMsgQueueItem>("wifi_dashboard_log", { .msg = "Connecting..." } );
    }
}
void WifiDashboard::wifi_got_ip_handler(const WifiGotIp& t_event) {
    ESP_LOGI(WifiDashboard::TAG, "Wifi got ip event from WifiDashboard: " IPSTR, IP2STR(&t_event.ip));
    m_app_state->m_queue_manager.post<LogMsgQueueItem>("wifi_dashboard_log", { .msg = "Got IP from WifiDashboard" } );
}

void WifiDashboard::init() {
    // register queues
    m_app_state->m_queue_manager.register_queue<SetState>("wifi_dashboard_state", 5);
    m_app_state->m_queue_manager.register_queue<LogMsgQueueItem>("wifi_dashboard_log", 5);

    // register callbacks 
    CALLBACK_CLASS_BIND(m_app_state->m_cb_manager, WifiEvent, WifiDashboard::wifi_event_handler);
    CALLBACK_CLASS_BIND(m_app_state->m_cb_manager, WifiGotIp, WifiDashboard::wifi_got_ip_handler);

    init_styles();

    m_screen = lv_obj_create(nullptr);
    lv_scr_load(m_screen);
    lv_obj_add_style(m_screen, &m_style_screen, 0);

    lv_obj_set_flex_flow(m_screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(m_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(m_screen, 0, 0);

    lv_obj_clear_flag(m_screen, LV_OBJ_FLAG_SCROLLABLE);  // disable scroll entirely

    init_header();
    init_status_panel();
    init_log_panel();
    init_footer();
}

void WifiDashboard::init_styles() {
    lv_style_init(&m_style_screen);
    lv_style_set_bg_color(&m_style_screen, CLR_BG);
    lv_style_set_bg_opa(&m_style_screen, LV_OPA_COVER);
    lv_style_set_pad_all(&m_style_screen, 0);
    lv_style_set_border_width(&m_style_screen, 0);

    lv_style_init(&m_style_header);
    lv_style_set_bg_color(&m_style_header, CLR_PANEL);
    lv_style_set_bg_opa(&m_style_header, LV_OPA_COVER);
    lv_style_set_border_width(&m_style_header, 1);
    lv_style_set_border_color(&m_style_header, CLR_BORDER);
    lv_style_set_border_side(&m_style_header, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_pad_hor(&m_style_header, 12);
    lv_style_set_pad_ver(&m_style_header, 8);
    lv_style_set_radius(&m_style_header, 0);

    lv_style_init(&m_style_panel);
    lv_style_set_bg_color(&m_style_panel, CLR_PANEL);
    lv_style_set_bg_opa(&m_style_panel, LV_OPA_COVER);
    lv_style_set_border_width(&m_style_panel, 1);
    lv_style_set_border_color(&m_style_panel, CLR_BORDER);
    lv_style_set_radius(&m_style_panel, 6);
    lv_style_set_pad_all(&m_style_panel, 8);

    lv_style_init(&m_style_log);
    lv_style_set_bg_color(&m_style_log, CLR_BG);
    lv_style_set_bg_opa(&m_style_log, LV_OPA_COVER);
    lv_style_set_text_color(&m_style_log, CLR_TEXT_MUTED);
    lv_style_set_text_font(&m_style_log, &lv_font_montserrat_10);
    lv_style_set_border_width(&m_style_log, 0);
    lv_style_set_pad_all(&m_style_log, 6);
    lv_style_set_radius(&m_style_log, 4);

    lv_style_init(&m_style_btn);
    lv_style_set_bg_color(&m_style_btn, CLR_BTN);
    lv_style_set_bg_opa(&m_style_btn, LV_OPA_COVER);
    lv_style_set_radius(&m_style_btn, 4);
    lv_style_set_border_width(&m_style_btn, 0);
    lv_style_set_shadow_width(&m_style_btn, 0);
    lv_style_set_pad_hor(&m_style_btn, 6);
    lv_style_set_pad_ver(&m_style_btn, 4);

    lv_style_init(&m_style_btn_pressed);
    lv_style_set_bg_color(&m_style_btn_pressed, CLR_BTN_PRESS);
    lv_style_set_transform_scale(&m_style_btn_pressed, 250);
}

void WifiDashboard::init_header() {
    m_header = lv_obj_create(m_screen);
    lv_obj_add_style(m_header, &m_style_header, 0);
    lv_obj_set_width(m_header, LV_PCT(100));
    lv_obj_set_height(m_header, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(m_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(m_header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(m_header, 8, 0);

    lv_obj_t* icon = lv_label_create(m_header);
    lv_label_set_text(icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(icon, CLR_ACCENT, 0);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0);

    lv_obj_t* title = lv_label_create(m_header);
    lv_label_set_text(title, "WiFi Dashboard");
    lv_obj_set_style_text_color(title, CLR_TEXT, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);

    lv_obj_t* spacer = lv_obj_create(m_header);
    lv_obj_set_size(spacer, 0, 0);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);

    lv_obj_t* sub = lv_label_create(m_header);
    lv_label_set_text(sub, "ESP32");
    lv_obj_set_style_text_color(sub, CLR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_10, 0);
}

void WifiDashboard::init_status_panel() {
    m_status_panel = lv_obj_create(m_screen);
    lv_obj_add_style(m_status_panel, &m_style_panel, 0);
    lv_obj_set_width(m_status_panel, LV_PCT(100));
    lv_obj_set_height(m_status_panel, LV_SIZE_CONTENT);
    lv_obj_set_style_margin_hor(m_status_panel, 6, 0);
    lv_obj_set_style_margin_top(m_status_panel, 6, 0);
    lv_obj_set_style_margin_bottom(m_status_panel, 4, 0);  // tight gap to log panel
    lv_obj_set_flex_flow(m_status_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(m_status_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(m_status_panel, 8, 0);

    lv_obj_t* section = lv_label_create(m_status_panel);
    lv_label_set_text(section, "STATUS");
    lv_obj_set_style_text_color(section, CLR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(section, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_letter_space(section, 1, 0);

    lv_obj_t* spacer = lv_obj_create(m_status_panel);
    lv_obj_set_size(spacer, 0, 0);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);

    m_status_icon = lv_obj_create(m_status_panel);
    lv_obj_set_size(m_status_icon, 8, 8);
    lv_obj_set_style_radius(m_status_icon, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(m_status_icon, 0, 0);

    m_status_label = lv_label_create(m_status_panel);
    lv_obj_set_style_text_color(m_status_label, CLR_TEXT, 0);
    lv_obj_set_style_text_font(m_status_label, &lv_font_montserrat_12, 0);

    apply_status_style();
}

void WifiDashboard::init_log_panel() {
    m_log_panel = lv_obj_create(m_screen);
    lv_obj_add_style(m_log_panel, &m_style_panel, 0);
    lv_obj_set_width(m_log_panel, LV_PCT(100));
    lv_obj_set_flex_grow(m_log_panel, 1);               // fills all remaining space
    lv_obj_set_style_margin_hor(m_log_panel, 6, 0);
    lv_obj_set_style_margin_bottom(m_log_panel, 2, 0);     // padding above footer
    lv_obj_set_flex_flow(m_log_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(m_log_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(m_log_panel, 6, 0);

    m_log_header = lv_label_create(m_log_panel);
    lv_label_set_text(m_log_header, "CONNECTION LOG");
    lv_obj_set_style_text_color(m_log_header, CLR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(m_log_header, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_letter_space(m_log_header, 1, 0);

    lv_obj_t* divider = lv_obj_create(m_log_panel);
    lv_obj_set_size(divider, LV_PCT(100), 1);
    lv_obj_set_style_bg_color(divider, CLR_BORDER, 0);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(divider, 0, 0);
    lv_obj_set_style_pad_all(divider, 0, 0);

    m_log_textarea = lv_textarea_create(m_log_panel);
    lv_obj_add_style(m_log_textarea, &m_style_log, 0);
    lv_obj_set_width(m_log_textarea, LV_PCT(100));
    lv_obj_set_flex_grow(m_log_textarea, 1);            // textarea fills log panel
    lv_textarea_set_one_line(m_log_textarea, false);
    lv_textarea_set_cursor_click_pos(m_log_textarea, false);
    lv_obj_clear_flag(m_log_textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_textarea_set_placeholder_text(m_log_textarea, "No log entries...");
}

void WifiDashboard::init_footer() {
    m_footer = lv_obj_create(m_screen);
    lv_obj_set_width(m_footer, lv_display_get_horizontal_resolution(NULL));
    lv_obj_set_height(m_footer, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(m_footer, CLR_PANEL, 0);
    lv_obj_set_style_bg_opa(m_footer, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(m_footer, 1, 0);
    lv_obj_set_style_border_color(m_footer, CLR_BORDER, 0);
    lv_obj_set_style_border_side(m_footer, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_radius(m_footer, 0, 0);
    lv_obj_set_style_pad_hor(m_footer, 4, 0);
    lv_obj_set_style_pad_ver(m_footer, 4, 0);
    lv_obj_clear_flag(m_footer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(m_footer, false, 0);
    lv_obj_set_flex_flow(m_footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(m_footer, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    m_reconnect_btn = lv_btn_create(m_footer);
    lv_obj_add_style(m_reconnect_btn, &m_style_btn, 0);
    lv_obj_add_style(m_reconnect_btn, &m_style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_set_style_margin_right(m_reconnect_btn, 4, 0);
    lv_obj_set_flex_flow(m_reconnect_btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(m_reconnect_btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(m_reconnect_btn, 2, 0);
    lv_obj_add_event_cb(m_reconnect_btn, on_reconnect_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* icon_lbl = lv_label_create(m_reconnect_btn);
    lv_label_set_text(icon_lbl, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_color(icon_lbl, CLR_TEXT, 0);
    lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_8, 0);

    m_reconnect_lbl = lv_label_create(m_reconnect_btn);
    lv_label_set_text(m_reconnect_lbl, "Reconnect");
    lv_obj_set_style_text_color(m_reconnect_lbl, CLR_TEXT, 0);
    lv_obj_set_style_text_font(m_reconnect_lbl, &lv_font_montserrat_10, 0);
}

// ─────────────────────────────────────────────
//  Status helpers
// ─────────────────────────────────────────────
void WifiDashboard::apply_status_style() {
    struct StatusMeta {
        const char*  label;
        lv_color_t   colour;
    };

    static const StatusMeta k_meta[] = {
        { "No Connection", /* CLR_ERR  */ lv_color_hex(0xDA3633) },
        { "Connecting...", /* CLR_WARN */ lv_color_hex(0xD29922) },
        { "Connected",     /* CLR_ACNT */ lv_color_hex(0x238636) },
    };

    const auto idx = static_cast<uint8_t>(m_status);
    lv_label_set_text(m_status_label, k_meta[idx].label);
    lv_obj_set_style_bg_color(m_status_icon, k_meta[idx].colour, 0);
    lv_obj_set_style_text_color(m_status_label, k_meta[idx].colour, 0);
}

void WifiDashboard::set_connection_status(ConnectionStatus t_status) {
    m_status = t_status;
    apply_status_style();
}

// ─────────────────────────────────────────────
//  Log helpers
// ─────────────────────────────────────────────
void WifiDashboard::add_log_message(const char* t_msg) {
    // Prepend timestamp placeholder — swap in real tick if available
    char buf[128];
    snprintf(buf, sizeof(buf), "[%6lu] %s", (long unsigned int)lv_tick_get() / 1000, t_msg);

    // If the textarea already has content, add a newline first
    const char* existing = lv_textarea_get_text(m_log_textarea);
    if (existing && existing[0] != '\0') {
        lv_textarea_add_char(m_log_textarea, '\n');
    }
    lv_textarea_add_text(m_log_textarea, buf);

    // Auto-scroll to bottom
    lv_obj_scroll_to_y(m_log_textarea, LV_COORD_MAX, LV_ANIM_ON);
}

// ─────────────────────────────────────────────
//  Static callback
// ─────────────────────────────────────────────
void WifiDashboard::on_reconnect_cb(lv_event_t* t_e) {
    // auto* self = static_cast<WifiDashboard*>(lv_event_get_user_data(t_e));
    // if (!self) return;
    //
    // self->add_log_message("Reconnect requested by user");
    // self->set_connection_status(ConnectionStatus::Connecting);
    //
    // // Hook into your reconnect logic here, e.g.:
    // // self->m_app_state->wifi.request_reconnect();
}
