#include "ssd1306.h"
#include "font.h"
#include <stdlib.h>
#include <string.h>

void ssd1306_init(ssd1306_t *display, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c) {
    display->width = width;
    display->height = height;
    display->total_pages = height / 8U;
    display->device_address = address;
    display->i2c_instance = i2c;
    display->framebuffer_size = display->total_pages * display->width + 1;
    display->framebuffer = calloc(display->framebuffer_size, sizeof(uint8_t));
    display->framebuffer[0] = 0x40;
    display->buffer[0] = 0x80;  // Ajuste para comando correto
}

void ssd1306_configure(ssd1306_t *display) {
    static const uint8_t init_sequence[] = {
        DISPLAY_OFF, MEMORY_ADDR_MODE, 0x01, DISPLAY_START_LINE | 0x00,
        SEGMENT_REMAP | 0x01, MULTIPLEX_RATIO, DISPLAY_HEIGHT - 1, COMMON_OUTPUT_DIR | 0x08,
        DISPLAY_OFFSET, 0x00, COMMON_PIN_CONFIG, 0x12, DISPLAY_CLOCK_DIV, 0x80,
        PRECHARGE_PERIOD, 0xF1, VCOM_DESELECT_LEVEL, 0x30, CONTRAST_CONTROL,
        0xFF, ENTIRE_DISPLAY_ON, NORMAL_INV_MODE, CHARGE_PUMP_CONTROL, 0x14, DISPLAY_ON
    };
    for (size_t i = 0; i < sizeof(init_sequence); i++) {
        ssd1306_send_command(display, init_sequence[i]);
    }
}

void ssd1306_send_command(ssd1306_t *display, uint8_t command) {
    display->buffer[1] = command;
    i2c_write_blocking(display->i2c_instance, display->device_address, display->buffer, 2, false);
}

void ssd1306_send_data(ssd1306_t *display) {
    ssd1306_send_command(display, COLUMN_ADDR);
    ssd1306_send_command(display, 0);
    ssd1306_send_command(display, display->width - 1);
    ssd1306_send_command(display, PAGE_ADDR);
    ssd1306_send_command(display, 0);
    ssd1306_send_command(display, display->total_pages - 1);
    i2c_write_blocking(display->i2c_instance, display->device_address, display->framebuffer, display->framebuffer_size, false);
}

void ssd1306_draw_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool state) {
    uint16_t index = (y >> 3) * display->width + x + 1;
    uint8_t pixel = (y & 0b111);
    if (state)
        display->framebuffer[index] |= (1 << pixel);
    else
        display->framebuffer[index] &= ~(1 << pixel);
}

void ssd1306_fill_screen(ssd1306_t *display, bool state) {
    memset(display->framebuffer + 1, state ? 0xFF : 0x00, display->framebuffer_size - 1);
}

void ssd1306_draw_char(ssd1306_t *display, char character, uint8_t x, uint8_t y) {
    uint8_t index = character - ' ';
    if (index < 0 || index >= sizeof(font_5x5) / sizeof(font_5x5[0])) {
        return;  
    }

    for (uint8_t i = 0; i < 5; ++i) {
        uint8_t line = font_5x5[index][i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (line & (1 << j)) {
                ssd1306_draw_pixel(display, x + i, y + j, true);
            }
        }
    }
}

void ssd1306_draw_text(ssd1306_t *display, const char *text, uint8_t x, uint8_t y) {
    while (*text) {
        ssd1306_draw_char(display, *text++, x, y);
        x += 6; // Largura de cada caractere
        if (x + 6 >= display->width) {
            x = 0;
            y += 8;
        }
        if (y + 8 >= display->height) {
            break;
        }
    }
}
