#pragma once

#include <cstdint>
#include "display/framebuffer.h"
#include "esp_lcd_types.h"

#define LCD_PIN_NUM_PCLK GPIO_NUM_18
#define LCD_PIN_NUM_MOSI GPIO_NUM_23
#define LCD_PIN_NUM_CS GPIO_NUM_5
#define LCD_PIN_NUM_DC GPIO_NUM_25
#define LCD_PIN_NUM_RST GPIO_NUM_26
#define LCD_PIN_NUM_BACKLIGHT GPIO_NUM_27
#define LCD_SPI_HOST SPI3_HOST

// range of values per color chanel
#define R_DEPTH 32
#define G_DEPTH 64
#define B_DEPTH 32

class LcdDisplay {
public:

    LcdDisplay(uint16_t t_width = 320, uint16_t t_height = 240, uint16_t t_draw_buf_lines = 5);
    ~LcdDisplay();

    inline uint16_t get_draw_buf_lines() const { return k_draw_buf_lines; }
    inline uint16_t get_width() const { return k_width; }
    inline uint16_t get_height() const { return k_height; }

    /**
     * Draws the current buffer onto the screen
     */
    void draw_framebuffer(const FrameBuffer& t_fb);
    /**
     * Draws buffer on screen for LVGL
     */
    void draw_area(int x1, int y1, int x2, int y2, const uint16_t* px_map);

    /**
     * Sets the backlight to a specific brightness
     *
     * For now, if != 0, set full brightness until LEDC manager is done
     */
    void set_backlight(uint8_t t_brightness);

    /* On/Off */
    void turn_on() const;
    void turn_off() const;

    /* Color */
    void invert_color(bool t_invert) const;

    /* Orientation */
    void mirror(bool t_mirror_x, bool t_mirror_y) const;
    void swap_xy(bool t_swap) const;

    /* Drawing */
    void set_gap(int t_x_gap, int t_y_gap) const;

private:

    void init_backlight();
    void init_display();

private:

    // data
    const uint16_t k_width;
    const uint16_t k_height;
    const uint16_t k_draw_buf_lines;

    // draw buffer
    uint16_t* m_draw_buf;

    // backlight
    uint8_t m_backlight;

    // esp handels
    esp_lcd_panel_handle_t m_panel_handle;
    esp_lcd_panel_io_handle_t m_io_handle;
};
