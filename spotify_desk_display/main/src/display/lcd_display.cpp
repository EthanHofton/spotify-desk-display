#include "display/lcd_display.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_panel_io.h"
#include "esp_heap_caps.h"

static const char* TAG = "LcdDisplay";

LcdDisplay::LcdDisplay(uint16_t t_width, uint16_t t_height, uint16_t t_draw_buf_lines) 
    : k_width(t_width), 
      k_height(t_height),
      k_draw_buf_lines(t_draw_buf_lines),
      m_draw_buf(nullptr),
      m_panel_handle(nullptr),
      m_io_handle(nullptr) {
    // setup backlight
    init_backlight();
    // setup display
    init_display();

    ESP_LOGI(TAG, "LCD Display configured");
}

LcdDisplay::~LcdDisplay() {
    ESP_LOGI(TAG, "Deleting buffer, freeing handels");

    if (m_draw_buf != nullptr) {
        heap_caps_free(m_draw_buf);
    }

    esp_lcd_panel_del(m_panel_handle);
    esp_lcd_panel_io_del(m_io_handle);
    spi_bus_free(LCD_SPI_HOST);
}

void LcdDisplay::set_backlight(uint8_t t_brightness) {
    m_backlight = t_brightness;
    gpio_set_level(LCD_PIN_NUM_BACKLIGHT, (m_backlight > 0) ? 1 : 0);
}

void LcdDisplay::draw_framebuffer(const FrameBuffer& t_fb) {
    // create display buffer
    if (m_draw_buf == nullptr) {
        size_t buf_size = k_width * k_draw_buf_lines * sizeof(uint16_t);
        m_draw_buf = (uint16_t*)heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
        if (!m_draw_buf) {
            ESP_LOGE(TAG, "Failed to allocate draw buffer (%u bytes)", (unsigned)buf_size);
            abort();
        }

        ESP_LOGI(TAG, "Created DMA compatable heap buffer for framebuffer rendering of bytes %u", buf_size);
    }

    uint16_t lines_left = k_height;
    uint16_t y = 0;
    const auto& buffer = t_fb.get_buffer();
    while (lines_left > 0) {
        int chunk = (lines_left < k_draw_buf_lines) ? lines_left : k_draw_buf_lines;
        for (uint16_t row = 0; row < chunk; row++) {
            for (uint16_t x = 0; x < k_width; x++) {
                m_draw_buf[row * k_width + x] = buffer[(y + row) * k_width + x].rgb565();
            }
        }

        esp_lcd_panel_draw_bitmap(m_panel_handle, 0, y, k_width,y + chunk, m_draw_buf);
        y += chunk;
        lines_left -= chunk;
    }
}

void LcdDisplay::draw_area(int x1, int y1, int x2, int y2, const uint16_t* px_map) {
    esp_lcd_panel_draw_bitmap(
        m_panel_handle,
        x1, y1,          // top-left (inclusive)
        x2 + 1, y2 + 1,  // bottom-right (exclusive for IDF driver)
        px_map
    );
}

void LcdDisplay::init_backlight() {
    // simple for now, wait until lecd manager is done
    gpio_set_direction(LCD_PIN_NUM_BACKLIGHT, GPIO_MODE_OUTPUT);
    set_backlight(1);
}

void LcdDisplay::init_display() {
    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t bus_config = {
        .mosi_io_num = LCD_PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = LCD_PIN_NUM_PCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = static_cast<int>(k_width * k_draw_buf_lines * sizeof(uint16_t)),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI bus initialised");

    ESP_LOGI(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = ILI9341_PANEL_IO_SPI_CONFIG(
        LCD_PIN_NUM_CS,
        LCD_PIN_NUM_DC,
        NULL,
        NULL
    );
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &m_io_handle));
    ESP_LOGI(TAG, "Pannel IO configured");

    ESP_LOGI(TAG, "Configure ILI9341 panel driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,  // RGB element order: R-G-B
        .bits_per_pixel = 16,                        // Implemented by LCD command `3Ah` (16/18)
        .reset_gpio_num = LCD_PIN_NUM_RST,   // Set to -1 if not use
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(m_io_handle, &panel_config, &m_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(m_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(m_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(m_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(m_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(m_panel_handle, true, true));
}

void LcdDisplay::turn_on() const {
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(m_panel_handle, true));
}

void LcdDisplay::turn_off() const {
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(m_panel_handle, false));
}

void LcdDisplay::invert_color(bool t_invert) const {
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(m_panel_handle, t_invert));
}

void LcdDisplay::mirror(bool t_mirror_x, bool t_mirror_y) const {
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(m_panel_handle, t_mirror_x, t_mirror_y));
}

void LcdDisplay::swap_xy(bool t_swap) const {
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(m_panel_handle, t_swap));
}

void LcdDisplay::set_gap(int t_x_gap, int t_y_gap) const {
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(m_panel_handle, t_x_gap, t_y_gap));
}
