#pragma once
#include "ui/base_page.h"

class HelloWorld : public BasePage {
public:
    HelloWorld(AppState* t_app_state, PageManager* t_page_manager);
    virtual ~HelloWorld() override;
    virtual void init() override;
    virtual void update() override;
    virtual void on_button_press(const PageManagerQueueItem& t_qitem) override;
};
