#pragma once

#include "tasks/app_state.h"

struct PageManagerQueueItem {
    char m_key[64];
    size_t m_press_timestamp;
    size_t m_last_press_timestamp;
};
