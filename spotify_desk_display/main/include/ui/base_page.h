#pragma once

#include "managers/lvgl_manager.h"
#include "tasks/app_state.h"
#include <memory>

class BasePage {
public:

    BasePage(AppState* t_app_state);
    virtual ~BasePage() = 0;

    void run();

    virtual void init() = 0;
    virtual void update() = 0;

protected:

    AppState* m_app_state;
    std::unique_ptr<LcdDisplay> m_display;
    std::unique_ptr<LvglManager> m_lvgl_manager;
};
