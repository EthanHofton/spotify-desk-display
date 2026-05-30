#include "ui/spotify_player_page.h"
#include "core/lv_obj.h"
#include "widgets/label/lv_label.h"
#include "ui/page_manager.h"
#include "widgets/spinner/lv_spinner.h"

// ── Spotify brand colours ──────────────────────────────────────────────────
static const lv_color_t SPOTIFY_GREEN   = lv_color_hex(0x1DB954);
static const lv_color_t SPOTIFY_BLACK   = lv_color_hex(0x191414);
static const lv_color_t SPOTIFY_WHITE   = lv_color_hex(0xFFFFFF);
static const lv_color_t SPOTIFY_SUBTEXT = lv_color_hex(0xB3B3B3);
static const lv_color_t SPOTIFY_GREY    = lv_color_hex(0x535353);

SpotifyPlayerPage::SpotifyPlayerPage(AppState* t_app_state, PageManager* t_page_manager)
    : BasePage(t_app_state, t_page_manager),
      m_screen(nullptr),
      m_status_label(nullptr),
      m_header_label(nullptr),
      m_state(State::LOADING) {}
 
SpotifyPlayerPage::~SpotifyPlayerPage() {}

void SpotifyPlayerPage::init() {
    // Create and load the root screen
    m_screen = lv_obj_create(nullptr);
    lv_screen_load(m_screen);
 
    // Black background
    lv_obj_set_style_bg_color(m_screen, SPOTIFY_BLACK, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(m_screen, LV_OPA_COVER, LV_PART_MAIN);
 
    // Kick off with the loading state
    set_loading();
}

void SpotifyPlayerPage::update() {
}

void SpotifyPlayerPage::on_button_press(const PageManagerQueueItem& t_qitem) {
    ESP_LOGI(TAG, "Spotify Player Page - button pressed");
    if (m_state == State::READY) {
        set_loading();
    } else {
        set_ready();
    }
}

// ── State: LOADING ─────────────────────────────────────────────────────────
void SpotifyPlayerPage::set_loading(const char* message) {
    m_state = State::LOADING;
    clear_screen();
 
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
 
    lv_label_set_text(m_status_label, message);
}
 
void SpotifyPlayerPage::update_loading_message(const char* message) {
    if (m_state != State::LOADING || m_status_label == nullptr) return;
    lv_label_set_text(m_status_label, message);
}
 
// ── State: READY ───────────────────────────────────────────────────────────
void SpotifyPlayerPage::set_ready() {
    m_state = State::READY;
    clear_screen();
 
    // ── Header bar ────────────────────────────────────────────────────────
    lv_obj_t* header = lv_obj_create(m_screen);
    lv_obj_set_size(header, LV_PCT(100), 32);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, SPOTIFY_BLACK, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN);
 
    // Green accent line at the bottom of the header
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_color(header, SPOTIFY_GREEN, LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 2, LV_PART_MAIN);
 
    m_header_label = lv_label_create(header);
    lv_label_set_text(m_header_label, "Now Playing");
    lv_obj_set_style_text_color(m_header_label, SPOTIFY_WHITE, LV_PART_MAIN);
    lv_obj_set_style_text_font(m_header_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_center(m_header_label);
 
    // ── Body (blank placeholder) ───────────────────────────────────────────
    lv_obj_t* body = lv_obj_create(m_screen);
    lv_obj_set_size(body, LV_PCT(100), LV_PCT(100) - 32);
    lv_obj_align(body, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(body, SPOTIFY_BLACK, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(body, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(body, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(body, 8, LV_PART_MAIN);
}
 
// ── clear_screen ───────────────────────────────────────────────────────────
void SpotifyPlayerPage::clear_screen() {
    // Delete all children of the screen, resetting it for the next state
    lv_obj_clean(m_screen);
    m_status_label  = nullptr;
    m_header_label  = nullptr;
}
