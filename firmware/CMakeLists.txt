# ============================================================================
#  IoT Edge Gateway - Firmware target
# ============================================================================

add_executable(iot_edge_gateway
    src/main.c
    src/temperature/temp_sensor.c
    src/pwm_ctrl/pwm_fan.c
    src/modbus/modbus_rtu.c
    src/oled/ssd1306.c
    src/sdcard/sdcard_logger.c
    src/sdcard/ff_stub.c
    src/wifi/wifi_manager.c
    src/http/http_client.c
    src/statemachine/fsm_global.c
)

# Includes
target_include_directories(iot_edge_gateway PRIVATE
    include
    src
)

# Librerias del Pico SDK
target_link_libraries(iot_edge_gateway PRIVATE
    pico_stdlib
    pico_cyw43_arch_lwip_poll      # CYW43439 + lwIP (modo polling)
    hardware_adc
    hardware_pwm
    hardware_uart
    hardware_i2c
    hardware_spi
    hardware_dma
    hardware_timer
    hardware_irq
    hardware_watchdog
)

# UART stdio para depurar por GPIO0/1 NO: lo usa Modbus. Usamos USB CDC.
pico_enable_stdio_usb(iot_edge_gateway 1)
pico_enable_stdio_uart(iot_edge_gateway 0)

# Genera .uf2, .bin, .hex
pico_add_extra_outputs(iot_edge_gateway)
