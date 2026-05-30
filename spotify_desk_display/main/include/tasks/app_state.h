#pragma once

#include "managers/callback_manager.h"
#include "queue/queue_manager.h"
#include <memory>

#define GET_APP_STATE(t_arg) static_cast<AppState*>(t_arg)

class PageManager;

struct AppState {
    std::unique_ptr<QueueManager> queue_manager;
    std::unique_ptr<CallbackManager> cb_manager;

    AppState() {
        queue_manager = std::make_unique<QueueManager>();
        cb_manager = std::make_unique<CallbackManager>();
    }
};
