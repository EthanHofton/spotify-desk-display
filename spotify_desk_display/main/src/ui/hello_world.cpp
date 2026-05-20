#include "ui/hello_world.h"
#include "core/lv_obj.h"
#include "widgets/label/lv_label.h"
#include "ui/page_manager.h"

static const char* TAG = "HelloWorld";

HelloWorld::HelloWorld(AppState* t_app_state, PageManager* t_page_manager)
    : BasePage(t_app_state, t_page_manager) {}

HelloWorld::~HelloWorld() {}

void HelloWorld::init() {
    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_screen_load(screen);

    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "Hello, World!");
    lv_obj_center(label);
}

void HelloWorld::update() {
}

void HelloWorld::on_button_press(const PageManagerQueueItem& t_qitem) {
    ESP_LOGI(TAG, "Hello world - button pressed");
    m_page_manager->show_page("dashboard");
}
