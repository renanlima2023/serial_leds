#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "num.h" 
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Declaração da função set_one_led
void set_one_led(uint8_t r, uint8_t g, uint8_t b); 

// Definições de pinos
#define BUTTON_A 5
#define BUTTON_B 6
#define GREEN_LED_PIN 11
#define BLUE_LED_PIN 12
#define DEBOUNCE_TIME 500
#define IS_RGBW false
#define WS2812_PIN 7
#define I2C_PORT i2c1
#define I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 15

// Flags de estado dos LEDs
bool blue_led_on = false;
bool green_led_on = false;
bool led_buffer[NUM_PIXELS]; // Buffer de LEDs

ssd1306_t ssd; // Variável do display OLED

// Funções de controle de LEDs e display
void turn_on_led(bool green, bool blue);
void toggle_led(uint8_t led_pin, bool *led_state, uint8_t color);
void copy_array(bool *dest, const bool *src);
void print_char_on_display(char c);
void print_number_on_leds(char n);

// Funções de inicialização
void irq_setup();
void led_setup();
void i2c_setup();
void ws2812_setup();

// Funções de controle do PIO
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

// Funções de interrupção
static void irq_handler(uint gpio, uint32_t events);

int main() {
    stdio_init_all(); // Inicialização do sistema de entrada/saída

    // Inicialização de componentes
    led_setup();
    i2c_setup();
    irq_setup();
    ws2812_setup();

    while (true) {
        if (stdio_usb_connected()) {
            char c;
            if (scanf("%c", &c) == 1) {
                printf("Caractere lido: %c\n", c);

                // Ações com base no caractere lido
                if (c == '.') {
                    set_one_led(0, 0, 0); // Desliga os LEDs
                    continue;
                }

                // Ignora caracteres não desejados
                if ((c < '0' || c > '9') && (c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) continue;

                // Exibe o caractere no display e na matriz de LEDs
                print_char_on_display(c);
                print_number_on_leds(c);
            }
        }
        sleep_ms(10);
    }
}

// Funções de inicialização de periféricos

void ws2812_setup() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    // Inicializa o programa do WS2812
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
    set_one_led(0, 0, 0); // Desliga todos os LEDs
}

void i2c_setup() {
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa o I2C a 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDR, I2C_PORT); // Inicializa o display OLED
    ssd1306_config(&ssd); // Configura o display OLED
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd); // Envia dados ao display
}

void led_setup() {
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);

    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
}

void irq_setup() {
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}

// Funções de interrupção

static void irq_handler(uint gpio, uint32_t events) {
    static uint32_t last_time_a = 0;
    uint32_t current_time_a = to_ms_since_boot(get_absolute_time());
    static uint32_t last_time_b = 0;
    uint32_t current_time_b = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A) {
        if (current_time_a - last_time_a < DEBOUNCE_TIME) return; 
        last_time_a = current_time_a;
        toggle_led(GREEN_LED_PIN, &green_led_on, 0);  // Alterna o estado do LED verde
        return;
    }

    if (current_time_b - last_time_b < DEBOUNCE_TIME) return; 
    last_time_b = current_time_b;
    toggle_led(BLUE_LED_PIN, &blue_led_on, 1);  // Alterna o estado do LED azul
}

void toggle_led(uint8_t led_pin, bool *led_state, uint8_t color) {
    ssd1306_fill(&ssd, false);
    if (*led_state) {
        ssd1306_draw_string(&ssd, "Led desligado", 27, 32);
        ssd1306_send_data(&ssd);
        *led_state = false;
        turn_on_led(false, false); // Desliga os LEDs
    } else {
        ssd1306_draw_string(&ssd, "Led ligado", 27, 32);
        ssd1306_send_data(&ssd);
        *led_state = true;
        if (color == 0) {
            turn_on_led(true, false); // Liga o LED verde
        } else {
            turn_on_led(false, true); // Liga o LED azul
        }
    }
}

void turn_on_led(bool green, bool blue) {
    gpio_put(GREEN_LED_PIN, green);
    gpio_put(BLUE_LED_PIN, blue);
}

void copy_array(bool *dest, const bool *src) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        dest[i] = src[i];
    }
}

void print_char_on_display(char c) {
    ssd1306_fill(&ssd, false);
    ssd1306_draw_char(&ssd, c, 64, 32);
    ssd1306_send_data(&ssd);
    sleep_ms(100);
}

void print_number_on_leds(char n) {
    printf("Imprimindo caractere %c\n", n);
    switch (n) {
        case '0': copy_array(led_buffer, zero); break;
        case '1': copy_array(led_buffer, one); break;
        case '2': copy_array(led_buffer, two); break;
        case '3': copy_array(led_buffer, three); break;
        case '4': copy_array(led_buffer, four); break;
        case '5': copy_array(led_buffer, five); break;
        case '6': copy_array(led_buffer, six); break;
        case '7': copy_array(led_buffer, seven); break;
        case '8': copy_array(led_buffer, eight); break;
        case '9': copy_array(led_buffer, nine); break;
        default: return;
    }

    set_one_led(0, 0, 100); 
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_one_led(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer[i]) {
            put_pixel(color);
        } else {
            put_pixel(0); // Desliga o LED
        }
    }
}
