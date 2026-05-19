idf_component_register(
    SRCS
        "spotify_desk_display.cpp"
        "src/managers/nvs_manager.cpp"
        "src/display/lcd_display.cpp"
        "src/display/framebuffer.cpp"
    INCLUDE_DIRS
        "include"
)

target_compile_options(${COMPONENT_LIB} PRIVATE 
    -std=gnu++20
    -Wno-missing-field-initializers
)
