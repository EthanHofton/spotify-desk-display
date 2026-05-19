#include "esp_log.h"
#include "managers/nvs_manager.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "freertos/FreeRTOS.h"
#include "esp_heap_caps.h"

static const char* TAG = "app_main";

#define EXAMPLE_PIN_NUM_LCD_PCLK GPIO_NUM_18
#define EXAMPLE_PIN_NUM_LCD_MOSI GPIO_NUM_23
#define EXAMPLE_PIN_NUM_LCD_CS GPIO_NUM_5
#define EXAMPLE_PIN_NUM_LCD_DC GPIO_NUM_25
#define EXAMPLE_PIN_NUM_LCD_RST GPIO_NUM_26
#define EXAMPLE_LCD_HOST SPI3_HOST
#define LCD_WIDTH  320
#define LCD_HEIGHT 240
#define LCD_DRAW_BUF_LINES   20                    /* lines per DMA push */

#define R_DEPTH 32
#define G_DEPTH 64
#define B_DEPTH 32

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    /* ILI9341 expects big-endian RGB565 over SPI */
    uint16_t v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    return (v >> 8) | (v << 8);   /* swap bytes for big-endian wire format */
}

/* ── Draw a horizontal gradient band ────────────────────────────
 *  Fills rows [y_start, y_start+height) with a linear colour blend
 *  from `col_left` to `col_right` (both in host-endian RGB565).
 *
 *  `buf`  – scratch buffer of at least (LCD_WIDTH * LCD_DRAW_BUF_LINES * 2) bytes
 * ─────────────────────────────────────────────────────────────── */
static void draw_gradient_band(esp_lcd_panel_handle_t panel,
                               uint16_t *buf,
                               int y_start, int height,
                               uint8_t r0, uint8_t g0, uint8_t b0,
                               uint8_t r1, uint8_t g1, uint8_t b1)
{
    int lines_left = height;
    int y = y_start;
 
    while (lines_left > 0) {
        int chunk = (lines_left < LCD_DRAW_BUF_LINES) ? lines_left : LCD_DRAW_BUF_LINES;
 
        /* Fill every pixel in this chunk of rows */
        for (int row = 0; row < chunk; row++) {
            for (int x = 0; x < LCD_WIDTH; x++) {
                /* Interpolate colour along X axis */
                uint8_t r = r0 + (int)(r1 - r0) * x / (LCD_WIDTH - 1);
                uint8_t g = g0 + (int)(g1 - g0) * x / (LCD_WIDTH - 1);
                uint8_t b = b0 + (int)(b1 - b0) * x / (LCD_WIDTH - 1);
                buf[row * LCD_WIDTH + x] = rgb565(r, g, b);
            }
        }
 
        esp_lcd_panel_draw_bitmap(panel,
                                  0, y,
                                  LCD_WIDTH, y + chunk,
                                  buf);
        y          += chunk;
        lines_left -= chunk;
    }
}

extern "C" void app_main(void) {
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_27, 1);

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t bus_config = {
        .mosi_io_num = EXAMPLE_PIN_NUM_LCD_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = EXAMPLE_PIN_NUM_LCD_PCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_DRAW_BUF_LINES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(EXAMPLE_LCD_HOST, &bus_config, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI bus initialised");

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = ILI9341_PANEL_IO_SPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS, EXAMPLE_PIN_NUM_LCD_DC,
                                                                                NULL, NULL);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EXAMPLE_LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install ILI9341 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;

    const esp_lcd_panel_dev_config_t panel_config = {
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,  // RGB element order: R-G-B
        .bits_per_pixel = 16,                        // Implemented by LCD command `3Ah` (16/18)
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,   // Set to -1 if not use
        // .vendor_config = &vendor_config,          // Uncomment this line if use custom initialization commands
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));

    size_t buf_bytes = LCD_WIDTH * LCD_DRAW_BUF_LINES * sizeof(uint16_t);
    uint16_t *draw_buf = (uint16_t*)heap_caps_malloc(buf_bytes, MALLOC_CAP_DMA);
    if (!draw_buf) {
        ESP_LOGE(TAG, "Failed to allocate draw buffer (%u bytes)", (unsigned)buf_bytes);
        return;
    }

    ESP_LOGI(TAG, "Drawing gradient 1: red → blue");
    draw_gradient_band(panel_handle, draw_buf,
                       0,            LCD_HEIGHT,
                       255, 0,   0,   /* start colour */
                       0,   0, 255);  /* end colour   */

    ESP_LOGI(TAG, "Done – gradients displayed");
    heap_caps_free(draw_buf);
}

void nvs_manager_test() {
    NvsManager nvs_manager = NvsManager::get_instance();

    std::string client_id = nvs_manager.get_str("spotify_creds", "client_id");
    ESP_LOGI(TAG, "Loaded spotify client id from nvs with NvsManager: %s", client_id.c_str());

    std::string client_secret = nvs_manager.get_str("spotify_creds", "client_secret");
    ESP_LOGI(TAG, "Loaded spotify client secret from nvs with NvsManager: %s", client_secret.c_str());

    std::string refresh_token = nvs_manager.get_str("spotify_creds", "refresh_token");
    ESP_LOGI(TAG, "Loaded spotify refresh token from nvs with NvsManager: %s", refresh_token.c_str());

    std::string token_expire = nvs_manager.get_str("spotify_creds", "token_expire");
    ESP_LOGI(TAG, "Loaded spotify token expire from nvs with NvsManager: %s", token_expire.c_str());
}
