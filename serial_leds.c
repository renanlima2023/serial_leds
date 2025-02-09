#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "num.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

#define BTN1 5
#define BTN2 6
#define LED1 11
#define LED2 12
#define DEBOUNCE_DELAY 500

#define RGBW_MODE false
#define LED_STRIP_PIN 7

#define OLED_I2C i2c1
#define OLED_ADDR 0x3C
#define SDA_PIN 14
#define SCL_PIN 15

ssd1306_t oled_display;

bool state_led1 = false;
bool state_led2 = false;
bool led_matrix[NUM_PIXELS];

void configure_leds();
void configure_i2c();
void configure_interrupts();
void configure_ws2812();
void toggle_led1();
void toggle_led2();
void copy_matrix(bool *destination, const bool *source);
static inline void send_pixel(uint32_t pixel_data);
static inline uint32_t encode_pixel(uint8_t r, uint8_t g, uint8_t b);
void illuminate_led(uint8_t r, uint8_t g, uint8_t b);
void display_character(char c);
void show_digit_on_leds(char n);
static void button_handler(uint gpio, uint32_t events);

int main() {
    stdio_init_all();

    configure_leds();
    configure_i2c();
    configure_interrupts();
    configure_ws2812();

    while (true) {
        if (stdio_usb_connected()) {
            char input_char;
            if (scanf("%c", &input_char) == 1) {
                printf("Input recebido: %c\n", input_char);

                if (input_char == '.') {
                    illuminate_led(0, 0, 0);
                    continue;
                }
                
                if (!((input_char >= '0' && input_char <= '9') ||
                      (input_char >= 'A' && input_char <= 'Z') ||
                      (input_char >= 'a' && input_char <= 'z'))) continue;

                display_character(input_char);
                show_digit_on_leds(input_char);
            }
        }
        sleep_ms(10);
    }
}

void configure_ws2812() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_STRIP_PIN, 800000, RGBW_MODE);
    illuminate_led(0, 0, 0);
}

void display_character(char c) {
    ssd1306_fill(&oled_display, false);
    ssd1306_draw_char(&oled_display, c, 64, 32);
    ssd1306_send_data(&oled_display);
}

void show_digit_on_leds(char n) {
    printf("Exibindo número %c\n", n);
    switch (n) {
        case '0': copy_matrix(led_matrix, zero); break;
        case '1': copy_matrix(led_matrix, one); break;
        case '2': copy_matrix(led_matrix, two); break;
        case '3': copy_matrix(led_matrix, three); break;
        case '4': copy_matrix(led_matrix, four); break;
        case '5': copy_matrix(led_matrix, five); break;
        case '6': copy_matrix(led_matrix, six); break;
        case '7': copy_matrix(led_matrix, seven); break;
        case '8': copy_matrix(led_matrix, eight); break;
        case '9': copy_matrix(led_matrix, nine); break;
        default: return;
    }
    illuminate_led(100, 0, 0);
}

void configure_interrupts() {
    gpio_set_irq_enabled_with_callback(BTN1, GPIO_IRQ_EDGE_FALL, true, &button_handler);
    gpio_set_irq_enabled_with_callback(BTN2, GPIO_IRQ_EDGE_FALL, true, &button_handler);
}
    
void configure_leds() {
    gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);

    gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);

    gpio_init(BTN1);
    gpio_set_dir(BTN1, GPIO_IN);
    gpio_pull_up(BTN1);

    gpio_init(BTN2);
    gpio_set_dir(BTN2, GPIO_IN);
    gpio_pull_up(BTN2);
}

void configure_i2c() {
    i2c_init(OLED_I2C, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init(&oled_display, WIDTH, HEIGHT, false, OLED_ADDR, OLED_I2C);
    ssd1306_config(&oled_display);
    ssd1306_fill(&oled_display, false);
    ssd1306_send_data(&oled_display);
}

void illuminate_led(uint8_t r, uint8_t g, uint8_t b) {
    gpio_put(LED1, r > 0);
    gpio_put(LED2, b > 0);
}

static void button_handler(uint gpio, uint32_t events) {
    static volatile uint32_t last_time1 = 0;
    volatile uint32_t now1 = to_ms_since_boot(get_absolute_time());
    static volatile uint32_t last_time2 = 0;
    volatile uint32_t now2 = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN1) {
        if (now1 - last_time1 < DEBOUNCE_DELAY) return;
        last_time1 = now1;
        printf("Botão 1 pressionado\n");
        toggle_led1();
        return;
    }

    if (now2 - last_time2 < DEBOUNCE_DELAY) return;
    last_time2 = now2;
    printf("Botão 2 pressionado\n");
    toggle_led2();
}

void toggle_led1() {
    ssd1306_fill(&oled_display, false);
    if (state_led1) {
        printf("Desativando LED 1\n");
        state_led1 = false;
        illuminate_led(0, 0, 0);
    } else {
        printf("Ativando LED 1\n");
        state_led1 = true;
        illuminate_led(1, 0, 0);
    }
    ssd1306_send_data(&oled_display);
}

void toggle_led2() {
    ssd1306_fill(&oled_display, false);
    if (state_led2) {
        printf("Desativando LED 2\n");
        state_led2 = false;
        illuminate_led(0, 0, 0);
    } else {
        printf("Ativando LED 2\n");
        state_led2 = true;
        illuminate_led(0, 0, 1);
    }
    ssd1306_send_data(&oled_display);
}

void copy_matrix(bool *destination, const bool *source) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        destination[i] = source[i];
    }
}
