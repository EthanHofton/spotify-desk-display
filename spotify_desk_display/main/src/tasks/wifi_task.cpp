#include "tasks/wifi_task.h"
#include "managers/wifi_manager.h"
#include "tasks/app_state.h"
#include "queue/wifi_command.h"

void wifi_task(void* t_arg) {
    AppState* app_state = GET_APP_STATE(t_arg);

    WifiManager manager(app_state);

    WifiCommand cmd;
    while (1) {
        if (app_state->m_queue_manager.poll(WifiManager::TAG, cmd)) {
            switch (cmd) {
                case WifiCommand::CONNECT: manager.connect(); break;
                case WifiCommand::DISCONNECT: manager.disconnect(); break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
