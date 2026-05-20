#pragma once

#include <memory>
#include <type_traits>
#include "ui/base_page.h"

template <typename T, typename = std::enable_if_t<std::is_base_of_v<BasePage, T>>>
void lvgl_task(void* t_arg) {
    AppState* app_state = GET_APP_STATE(t_arg);
    auto page = std::make_unique<T>(app_state);
    page->run();
}
