#include "tasks/lvgl_task.h"
#include "tasks/app_state.h"
#include "ui/hello_world.h"
#include "ui/page_manager.h"
#include "ui/wifi_dashboard.h"

void lvgl_task(void* t_arg) {
    AppState* app_state = GET_APP_STATE(t_arg);


    PageManager page_manager = PageManager(app_state);
    page_manager.register_button("main", GPIO_NUM_14);
    page_manager.register_page<WifiDashboard>("dashboard");
    page_manager.register_page<HelloWorld>("hello_world");

    page_manager.show_page("dashboard");

    page_manager.run();
}
