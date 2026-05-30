idf_component_register(
    SRCS
        # main
        "spotify_desk_display.cpp"

        # display
        "src/display/lcd_display.cpp"
        "src/display/framebuffer.cpp"

        # managers
        "src/managers/lvgl_manager.cpp"
        "src/managers/nvs_manager.cpp"
        "src/managers/wifi_manager.cpp"

        # ui
        "src/ui/base_page.cpp"
        "src/ui/wifi_dashboard.cpp"
        "src/ui/page_manager.cpp"
        "src/ui/hello_world.cpp"
        "src/ui/spotify_player_page.cpp"
        "src/ui/loading_page.cpp"

        # queue
        "src/queue/queue_manager.cpp"

        # tasks
        "src/tasks/lvgl_task.cpp"
        "src/tasks/wifi_task.cpp"
        "src/tasks/spotify_task.cpp"

        # http_client
        "src/http_client/http_client.cpp"

        # spotify client
        "src/spotify_client/spotify_client.cpp"
        "src/spotify_client/spotify_player_state.cpp"


        # utils
        "src/utils/encoding.cpp"

    INCLUDE_DIRS
        "include"
)

target_compile_options(${COMPONENT_LIB} PRIVATE 
    -std=gnu++20
    -Wno-missing-field-initializers
)
