#pragma once

#include "tasks/app_state.h"
#include "queue/page_manager_queue_item.h"

class PageManager;

class BasePage {
public:

    BasePage(AppState* t_app_state, PageManager* m_page_manager);
    virtual ~BasePage() = 0;

    virtual void init() = 0;
    virtual void update() = 0;
    virtual void on_button_press(const PageManagerQueueItem& t_qitem);

    void register_callback(CallbackToken t_token);

protected:

    AppState* m_app_state;
    PageManager* m_page_manager;

private:

    std::vector<CallbackToken> m_callback_tokens;
};
