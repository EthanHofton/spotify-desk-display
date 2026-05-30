#include "tasks/lvgl_task.h"
#include "tasks/app_state.h"
#include "ui/loading_page.h"
#include "ui/page_manager.h"
#include "ui/spotify_player_page.h"
#include "ui/wifi_dashboard.h"

void lvgl_task(void* t_arg) {
    AppState* app_state = GET_APP_STATE(t_arg);

    std::unique_ptr<PageManager> page_manager = std::make_unique<PageManager>(app_state);
    page_manager->register_button("main", GPIO_NUM_14);
    page_manager->register_page<LoadingPage>(LoadingPage::TAG);
    // page_manager.register_page<SpotifyPlayerPage>("spotify_player_page");

    page_manager->show_page(LoadingPage::TAG);

    page_manager->run();
}
