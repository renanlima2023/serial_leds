# Projeto Raspberry Pi Pico W - Controle de LEDs e Display OLED

Este projeto utiliza o Raspberry Pi Pico W para controlar LEDs, um display OLED e uma matriz de LEDs WS2812. O código implementa funcionalidades como:

- Controle de LEDs através de botões.
- Exibição de caracteres em um display OLED.
- Controle de uma matriz de LEDs WS2812.
- Leitura de caracteres via USB e exibição no display e na matriz de LEDs.

## Requisitos

- **Raspberry Pi Pico W**
- **WS2812 RGB LEDs**
- **Display OLED SSD1306**
- **Botões para controle de LEDs**
- **Compilador C (como o GCC para ARM)**
- **SDK do Raspberry Pi Pico**

## Instalação

1. **Configuração do ambiente:**
   - Instale o SDK do Raspberry Pi Pico em seu ambiente de desenvolvimento.
   - Certifique-se de que as bibliotecas necessárias para o display SSD1306 e a matriz de LEDs WS2812 estão disponíveis.

2. **Conexões do hardware:**
   - Conecte o display OLED aos pinos I2C (SDA no pino 14, SCL no pino 15).
   - Conecte os LEDs RGB WS2812 ao pino GPIO 7.
   - Conecte os botões A e B aos pinos GPIO 5 e GPIO 6, respectivamente.
   - Conecte o Raspberry Pi Pico W a um computador via USB para depuração e controle.

3. **Compilação:**
   - Clone este repositório.
   - Compile o código usando o ambiente de desenvolvimento configurado.

## Funcionalidades

### LEDs de Status
O código controla dois LEDs (verde e azul) conectados aos pinos GPIO 11 e 12. O estado desses LEDs pode ser alternado pressionando os botões A e B.

- **Botão A (GPIO 5):** Alterna o LED verde.
- **Botão B (GPIO 6):** Alterna o LED azul.

### Display OLED SSD1306
O display OLED exibe caracteres enviados via USB. Quando um caractere é lido, ele é exibido no display e na matriz de LEDs.

- **Leitura de caracteres:** O código lê caracteres via USB e exibe no display OLED e na matriz de LEDs WS2812.

### Matriz de LEDs WS2812
A matriz de LEDs é controlada para exibir números de 0 a 9. Cada número é mapeado para um padrão de LEDs predefinido, e o padrão é exibido na matriz de LEDs quando o número é lido via USB.

- **Exibição de números:** Os números de 0 a 9 são exibidos na matriz de LEDs de acordo com a entrada via USB.

### Controle de LEDs via USB
Quando um caractere é recebido via USB, o sistema verifica se o caractere está entre '0' e '9'. Se estiver, ele exibe o número na matriz de LEDs. Se o caractere for '.', os LEDs serão desligados.

## Estrutura do Código

- **Funções principais:**
  - `main()`: Função principal que configura os periféricos e gerencia a leitura dos caracteres via USB.
  - `irq_handler()`: Função de interrupção que alterna os LEDs quando os botões A e B são pressionados.
  - `turn_on_led()`: Função para controlar o estado dos LEDs (verde e azul).
  - `set_one_led()`: Função para controlar a matriz de LEDs WS2812.

- **Funções de inicialização:**
  - `ws2812_setup()`: Inicializa o controlador WS2812.
  - `i2c_setup()`: Configura a comunicação I2C para o display OLED.
  - `led_setup()`: Configura os LEDs e botões.
  - `irq_setup()`: Configura as interrupções para os botões A e B.

- **Funções de controle:**
  - `copy_array()`: Copia os padrões de LEDs para o buffer.
  - `print_char_on_display()`: Exibe um caractere no display OLED.
  - `print_number_on_leds()`: Exibe um número na matriz de LEDs.

## Como Usar

1. Compile o código e carregue no Raspberry Pi Pico W.
2. Conecte o Pico ao seu computador via USB.
3. Abra um terminal para interagir com o dispositivo.
4. Envie um caractere através do terminal para visualizá-lo no display OLED e na matriz de LEDs.
5. Pressione os botões A e B para alternar os LEDs de status.


