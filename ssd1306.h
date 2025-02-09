#ifndef SSD1306_H
#define SSD1306_H

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

typedef enum {
    CONTRAST_CONTROL = 0x81,
    ENTIRE_DISPLAY_ON = 0xA4,
    NORMAL_INV_MODE = 0xA6,
    DISPLAY_OFF = 0xAE,
    DISPLAY_ON = 0xAF, // Adicionado para corrigir erro
    MEMORY_ADDR_MODE = 0x20,
    COLUMN_ADDR = 0x21,
    PAGE_ADDR = 0x22,
    DISPLAY_START_LINE = 0x40,
    SEGMENT_REMAP = 0xA0,
    MULTIPLEX_RATIO = 0xA8,
    COMMON_OUTPUT_DIR = 0xC0,
    DISPLAY_OFFSET = 0xD3,
    COMMON_PIN_CONFIG = 0xDA,
    DISPLAY_CLOCK_DIV = 0xD5,
    PRECHARGE_PERIOD = 0xD9,
    VCOM_DESELECT_LEVEL = 0xDB,
    CHARGE_PUMP_CONTROL = 0x8D
} oled_command_t;

typedef struct {
    uint8_t width, height, total_pages, device_address;
    i2c_inst_t *i2c_instance;
    bool is_external_vcc;
    uint8_t *framebuffer;
    size_t framebuffer_size;
    uint8_t buffer[2];
} ssd1306_t;

void ssd1306_init(ssd1306_t *display, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c);
void ssd1306_configure(ssd1306_t *display);
void ssd1306_send_command(ssd1306_t *display, uint8_t command);
void ssd1306_send_data(ssd1306_t *display);

void ssd1306_draw_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool state);
void ssd1306_fill_screen(ssd1306_t *display, bool state);
void ssd1306_draw_rectangle(ssd1306_t *display, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool state, bool filled);
void ssd1306_draw_line(ssd1306_t *display, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool state);
void ssd1306_draw_horizontal_line(ssd1306_t *display, uint8_t x0, uint8_t x1, uint8_t y, bool state);
void ssd1306_draw_vertical_line(ssd1306_t *display, uint8_t x, uint8_t y0, uint8_t y1, bool state);
void ssd1306_draw_char(ssd1306_t *display, char character, uint8_t x, uint8_t y);
void ssd1306_draw_text(ssd1306_t *display, const char *text, uint8_t x, uint8_t y);

#endif /* SSD1306_H */
