# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# ====================================================================================

cmake_minimum_required(VERSION 3.13)

# Definição do padrão de C e C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Defina o board (placa) para o Pico W
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Caminho para o SDK do Raspberry Pi Pico (modifique conforme necessário)
set(PICO_SDK_PATH "C:/Users/renan/.pico-sdk")  # Caminho correto para o SDK

# Puxe o SDK do Raspberry Pi Pico
include(C:/Users/renan/.pico-sdk/cmake/pico_sdk_import.cmake)

# Nome do projeto
project(serial_leds C CXX ASM)

# Inicialize o SDK do Pico
pico_sdk_init()

# Adicione o executável principal
add_executable(serial_leds serial_leds.c ssd1306.c)

# Gere automaticamente o cabeçalho ws2812.pio.h
pico_generate_pio_header(serial_leds ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Nome e versão do programa
pico_set_program_name(serial_leds "serial_leds")
pico_set_program_version(serial_leds "0.1")

# Ative UART e USB para comunicação
pico_enable_stdio_uart(serial_leds 0)
pico_enable_stdio_usb(serial_leds 1)

# Adicione bibliotecas necessárias
target_link_libraries(serial_leds
    pico_stdlib
    hardware_pio
    hardware_timer
    hardware_i2c
)

# Adicione as dependências do PIO e configure corretamente
target_include_directories(serial_leds PRIVATE ${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include)
target_link_libraries(serial_leds hardware_pio)

# Inclua os diretórios necessários (já coberto pelo pico_sdk_import.cmake)
# target_include_directories(serial_leds PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Adicione saídas extras
pico_add_extra_outputs(serial_leds)
