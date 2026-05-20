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

        # ui
        "src/ui/base_page.cpp"
        "src/ui/wifi_dashboard.cpp"

        # tasks

    INCLUDE_DIRS
        "include"
)

target_compile_options(${COMPONENT_LIB} PRIVATE 
    -std=gnu++20
    -Wno-missing-field-initializers
)
