#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "num.h" 
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

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

ssd1306_t ssd;

bool blue_led_on = false;
bool green_led_on = false;
bool led_buffer[NUM_PIXELS]; // Buffer de LEDs

void turn_on_led(bool g, bool b);
static void irq_handler(uint gpio, uint32_t events);
void toggle_led(uint8_t led_pin, bool *led_state, uint8_t color);
void copy_array(bool *dest, const bool *src); // Função para copiar um array para outro
static inline void put_pixel(uint32_t pixel_grb); // Função para enviar um pixel para o buffer
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b); // Função para converter um pixel para um inteiro
void set_one_led(uint8_t r, uint8_t g, uint8_t b); // Função para definir a cor de todos os LEDs
void i2c_setup(); // Função para inicializar o I2C
void led_setup(); // Função para inicializar os LEDs
void irq_setup(); // Função para inicializar os IRQs
void ws2812_setup(); // Função para inicializar o WS2812
void print_number_on_leds(char n); // Função para imprimir um número na matriz de LEDs
void print_char_on_display(char c); // Função para imprimir um caractere no display OLED

int main()
{
    stdio_init_all();

    led_setup();
    i2c_setup();
    irq_setup();
    ws2812_setup();

    while (true) {
        if (stdio_usb_connected()) {
            char c;
            if (scanf("%c", &c) == 1) {
                printf("Caractere lido: %c\n", c);

                if (c == '.') {
                    set_one_led(0, 0, 0);
                    continue;
                }

                if ((c < '0' || c > '9') && (c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) continue;

                print_char_on_display(c);
                print_number_on_leds(c);
            }
        }
        sleep_ms(10);
    }
}

void ws2812_setup() {
    // Inicialização do PIO
    PIO pio = pio0;
    int sm = 0;

    uint offset = pio_add_program(pio, &ws2812_program);

    // Inicialização do programa do WS2812
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    set_one_led(0, 0, 0); // Desliga todos os LEDs da matriz
}

void print_char_on_display(char c) {
    ssd1306_fill(&ssd, false);
    ssd1306_draw_char(&ssd, c, 64, 32);
    ssd1306_send_data(&ssd);
}

void print_number_on_leds(char n) {
    printf("Imprimindo o número %c\n", n);
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

    set_one_led(100, 0, 0);
}

void irq_setup() {
    // Configuração do IRQ para o botão A
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &irq_handler);

    // Configuração do IRQ para o botão B
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}
    
void led_setup() {
    // Configuração do pino de LED azul
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);

    // Configuração do pino de LED verde
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);

    // Configuração do pino de botão A
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Configuração do pino de botão B
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
}

void i2c_setup() {
    // Inicialização do I2C em 400 kHz
    i2c_init(I2C_PORT, 400 * 1000);

    // Configuração dos pinos SDA e SCL
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDR, I2C_PORT); // Inicializa o display OLED
    ssd1306_config(&ssd); // Configura o display OLED
    ssd1306_fill(&ssd, false); // Limpa o display OLED
    ssd1306_send_data(&ssd); // Envia os dados para o display OLED
}

void turn_on_led(bool g, bool b) {
    gpio_put(GREEN_LED_PIN, g);
    gpio_put(BLUE_LED_PIN, b);
}

static void irq_handler(uint gpio, uint32_t events) {
    static uint32_t last_time_a = 0; // Tempo da última pressão do botão A
    uint32_t current_time_a = to_ms_since_boot(get_absolute_time()); // Tempo atual da pressão do botão A
    static uint32_t last_time_b = 0; // Tempo da última pressão do botão B
    uint32_t current_time_b = to_ms_since_boot(get_absolute_time()); // Tempo atual da pressão do botão B

    // Verifica se o botão pressionado foi o A
    if (gpio == BUTTON_A) {
        if (current_time_a - last_time_a < DEBOUNCE_TIME) return; // Verifica se o tempo de debounce foi atingido
        last_time_a = current_time_a;

        printf("Botão A pressionado\n");
        toggle_led(GREEN_LED_PIN, &green_led_on, 0);  // Aciona a função para alternar o estado do LED verde
        return;
    }

    // Se chegou aqui, o botão pressionado foi o B
    if (current_time_b - last_time_b < DEBOUNCE_TIME) return; // Verifica se o tempo de debounce foi atingido
    last_time_b = current_time_b;

    printf("Botão B pressionado\n");
    toggle_led(BLUE_LED_PIN, &blue_led_on, 1);  // Aciona a função para alternar o estado do LED azul
}

void toggle_led(uint8_t led_pin, bool *led_state, uint8_t color) {
    ssd1306_fill(&ssd, false);
    if (*led_state) { // Se o LED estiver ligado, desliga
        ssd1306_draw_string(&ssd, "Led", 27, 32);
        ssd1306_draw_string(&ssd, "desligado", 27, 42);
        ssd1306_send_data(&ssd);
        *led_state = false;
        turn_on_led(false, false);
    } else { // Se o LED estiver desligado, liga
        ssd1306_draw_string(&ssd, "Led", 27, 32);
        ssd1306_draw_string(&ssd, "ligado", 25, 42);
        ssd1306_send_data(&ssd);
        *led_state = true;
        if (color == 0) {  // LED verde
            turn_on_led(true, false);
        } else {           // LED azul
            turn_on_led(false, true);
        }
    }
}

void copy_array(bool *dest, const bool *src) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        dest[i] = src[i];
    }
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_one_led(uint8_t r, uint8_t g, uint8_t b) {
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer[i]) {
            put_pixel(color); // Liga o LED com um no buffer
        } else {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}
