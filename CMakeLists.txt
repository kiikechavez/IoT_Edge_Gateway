
#  IoT Edge Gateway - Build root para Raspberry Pi Pico SDK
#  Autor: Brando Enrique Chavez Vergara

cmake_minimum_required(VERSION 3.13)

# Definir la placa: Pico W
set(PICO_BOARD pico_w)

# Importar el SDK (debe existir la variable PICO_SDK_PATH)
include(pico_sdk_import.cmake)

project(iot_edge_gateway C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializar el SDK
pico_sdk_init()

# Construir el firmware
add_subdirectory(firmware)
