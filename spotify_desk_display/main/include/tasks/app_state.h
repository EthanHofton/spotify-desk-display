#pragma once

#include "managers/callback_manager.h"
#include "queue/queue_manager.h"

#define GET_APP_STATE(t_arg) static_cast<AppState*>(t_arg)

class PageManager;

struct AppState {
    AppState() = default;
    AppState(const AppState&) = delete;
    AppState& operator=(const AppState&) = delete;

    QueueManager m_queue_manager;
    CallbackManager m_cb_manager;
};
