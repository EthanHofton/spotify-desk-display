#pragma once

#include "display/lcd_display.h"
#include "display/lv_display.h"

class LvglManager {
public:

    LvglManager(LcdDisplay& t_display, uint16_t t_draw_buff_lines = 5);
    ~LvglManager();

    void render(const lv_area_t* area, uint8_t* px_buff);

    lv_display_t* lv_display() const { return m_lv_display; }

private:

    LcdDisplay&    m_display;
    uint16_t       m_draw_buff_lines;
    lv_display_t*  m_lv_display = nullptr;
    lv_color_t*       m_draw_buf1  = nullptr; // primary draw buffer
    lv_color_t*       m_draw_buf2  = nullptr;
};
