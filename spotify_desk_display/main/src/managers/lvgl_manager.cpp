#include "managers/lvgl_manager.h"
#include "display/lv_display.h"
#include "esp_heap_caps.h"
#include "freertos/idf_additions.h"
#include "lv_init.h"
#include "misc/lv_color.h"
#include "tick/lv_tick.h"
#include "esp_log.h"

static const char *TAG = "LvglManager";

void lv_flush_callback(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf) {
    auto* self = static_cast<LvglManager*>(lv_display_get_user_data(disp));
    self->render(area, px_buf);
}

static uint32_t lv_tick_cb(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

LvglManager::LvglManager(LcdDisplay& t_display, uint16_t t_draw_buff_lines) 
    : m_display(t_display), m_draw_buff_lines(t_draw_buff_lines) {
    lv_init();

    // Allocate draw buffers in DMA-capable SRAM ----------------------------
    //    Each buffer covers (width × buf_lines) RGB565 pixels.
    //    Two buffers allow LVGL to render the next chunk while the previous
    //    one is being transferred (double-buffering / partial mode).
    const size_t buf_bytes =
        static_cast<size_t>(m_display.get_width()) *
        static_cast<size_t>(m_draw_buff_lines) *
        sizeof(lv_color_t);  // RGB565 = 2 bytes per pixel

    m_draw_buf1 = static_cast<lv_color_t*>(
        heap_caps_malloc(buf_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    m_draw_buf2 = static_cast<lv_color_t*>(
        heap_caps_malloc(buf_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));

    ESP_LOGI(TAG, "Created double buffer for LVGL (%d per line of %d lines), 2 x %d byes = %d bytes",
        m_display.get_width(),
        m_draw_buff_lines,
        buf_bytes,
        buf_bytes * 2
    );

    if (!m_draw_buf1) {
        ESP_LOGE(TAG, "Failed to allocate primary draw buffer (%u bytes)", 
                 static_cast<unsigned>(buf_bytes));
        abort();
    }

    if (!m_draw_buf2) {
        ESP_LOGW(TAG, "Failed to allocate secondary buffer, falling back to single buffer");
        // m_draw_buf2 stays nullptr — lv_display_set_buffers accepts this fine
    }

    // Create LVGL display object -------------------------------------------
    m_lv_display = lv_display_create(m_display.get_width(), m_display.get_height());
    if (!m_lv_display) {
        ESP_LOGE(TAG, "lv_display_create() failed");
        abort();
    }

    // Register draw buffers ------------------------------------------------
    //    LV_DISPLAY_RENDER_MODE_PARTIAL — LVGL renders dirty rectangles into
    //    the buffer and calls flush_cb once per rectangle.
    lv_display_set_buffers(
        m_lv_display,
        m_draw_buf1,
        m_draw_buf2,   // nullptr is fine if second alloc failed
        buf_bytes,
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    // Register flush callback and store back-pointer -----------------------
    lv_display_set_user_data(m_lv_display, this);
    lv_tick_set_cb(lv_tick_cb);
    lv_display_set_flush_cb(m_lv_display, lv_flush_callback);
    lv_display_set_color_format(m_lv_display, LV_COLOR_FORMAT_RGB565_SWAPPED);
 
    ESP_LOGI(TAG, "LVGL display driver ready (%ux%u, %u-line buffer)",
         m_display.get_width(), m_display.get_height(), m_display.get_draw_buf_lines());
}

LvglManager::~LvglManager() {
    if (m_lv_display) {
        lv_display_delete(m_lv_display);
    }
    heap_caps_free(m_draw_buf1);
    heap_caps_free(m_draw_buf2);
    lv_deinit();
}

void LvglManager::render(const lv_area_t* area, uint8_t* px_buff) {
    m_display.draw_area(
        area->x1, area->y1,
        area->x2, area->y2,
        reinterpret_cast<const uint16_t*>(px_buff)
    );

    // Tell LVGL the flush is complete so it can queue the next rectangle.
    // If you switch to interrupt/DMA-completion driven flushing, move this
    // call to your DMA-done ISR / callback instead.
    lv_display_flush_ready(m_lv_display);
}

