#pragma once

#include "ui/base_page.h"

class SpotifyPlayerPage : public BasePage {
public:
    static constexpr const char* TAG = "SpotifyPlayerPage";

    SpotifyPlayerPage(AppState* t_app_state, PageManager* t_page_manager);
    virtual ~SpotifyPlayerPage() override;

    virtual void init() override;
    virtual void update() override;
    virtual void on_button_press(const PageManagerQueueItem& t_qitem) override;
};
