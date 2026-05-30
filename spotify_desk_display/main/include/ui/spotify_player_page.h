#pragma once

#include "misc/lv_types.h"
#include "ui/base_page.h"

class SpotifyPlayerPage : public BasePage {
public:
    static constexpr const char* TAG = "SpotifyPlayerPage";

    SpotifyPlayerPage(AppState* t_app_state, PageManager* t_page_manager);
    virtual ~SpotifyPlayerPage() override;

    virtual void init() override;
    virtual void update() override;
    virtual void on_button_press(const PageManagerQueueItem& t_qitem) override;

private:

    void set_loading(const char* message = "Loading...");   // Full-screen loading splash
    void update_loading_message(const char* message);       // Update label only, spinner untouched

    void set_ready();

    enum class State { LOADING, READY };
 
    void clear_screen();  // Wipes all child widgets from m_screen
 
    lv_obj_t* m_screen;
    lv_obj_t* m_status_label;   // Used in LOADING state
    lv_obj_t* m_header_label;   // Used in READY state
 
    State m_state;
};
