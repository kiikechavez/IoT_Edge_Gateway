/**
 * ============================================================================
 *  IoT Edge Gateway - Nodo de Edge Computing y Gateway IoT
 *  para Monitorizacion Industrial
 * ----------------------------------------------------------------------------
 *  Archivo : main.c
 *  Autor   : Brando Enrique Chavez Vergara
 *  Materia : Digitales 3 - Universidad de Antioquia (UdeA)
 *  SDK     : Raspberry Pi Pico SDK (C/C++)
 *  MCU     : Raspberry Pi Pico W (RP2040 + CYW43439)
 * ----------------------------------------------------------------------------
 *  Descripcion:
 *    Punto de entrada del firmware. Inicializa todos los subsistemas e
 *    implementa un super-loop con el patron "polling + interrupciones"
 *    (unico flujo de programa autorizado por la directiva del profesor).
 *
 *    Las ISRs son cortas y solo modifican flags 'volatile'. El super-loop
 *    sondea esas flags y delega el trabajo a los servicios de cada modulo.
 *
 *    La maquina de estados global (fsm_global) coordina el sistema en sus
 *    cuatro estados: INIT, ONLINE, OFFLINE_LOG, ERROR.
 * ============================================================================
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "config.h"
#include "temperature/temp_sensor.h"
#include "pwm_ctrl/pwm_fan.h"
#include "modbus/modbus_rtu.h"
#include "oled/ssd1306.h"
#include "sdcard/sdcard_logger.h"
#include "wifi/wifi_manager.h"
#include "http/http_client.h"
#include "statemachine/fsm_global.h"

/* ----------------------------------------------------------------------------
 *  Flags 'volatile' compartidas con las ISRs.
 *  Documentadas en el PDF de arquitectura (pregunta 5).
 * --------------------------------------------------------------------------*/
volatile bool g_flag_modbus_rx_done   = false;  /* ISR UART0 + timer 3.5char */
volatile bool g_flag_pwm_tick         = false;  /* ISR PWM wrap (100 Hz)     */
volatile bool g_flag_oled_refresh     = false;  /* timer suave 5 Hz          */
volatile bool g_flag_http_due         = false;  /* timer suave 1 Hz          */
volatile bool g_flag_sd_write_pending = false;  /* habilitada por FSM        */

/* ----------------------------------------------------------------------------
 *  Buffer de telemetria global (ultimas lecturas validas).
 *  Lo escriben los servicios; lo leen el HTTP cliente, el OLED y el SD logger.
 * --------------------------------------------------------------------------*/
telemetry_t g_telemetry = {0};

/* ----------------------------------------------------------------------------
 *  Programa principal: super-loop polling + interrupciones
 * --------------------------------------------------------------------------*/
int main(void) {
    /* --- Inicializacion de stdio (USB CDC) --- */
    stdio_init_all();
    sleep_ms(2000);
    printf("\n============================================\n");
    printf("  IoT Edge Gateway - Firmware iniciando\n");
    printf("  SDK: Raspberry Pi Pico SDK (C/C++)\n");
    printf("============================================\n\n");

    /* --- Inicializacion de subsistemas en orden de dependencia --- */
    fsm_global_init();                       /* FSM en estado INIT          */
    temp_sensor_init();                      /* ADC4 sensor termico interno */
    pwm_fan_init();                          /* PWM GP15, 25 kHz            */
    oled_init();                             /* I2C0 SSD1306                */
    modbus_init();                           /* UART0 + DMA0/DMA1 + timer   */
    sdcard_init();                           /* SPI1 + FatFs                */

    bool wifi_ok = wifi_manager_init();      /* CYW43 + lwIP                */
    if (!wifi_ok) {
        printf("[main] Wi-Fi init fallo. Entrando en OFFLINE_LOG.\n");
        fsm_global_set_event(EV_INIT_FAIL_WIFI);
    } else {
        fsm_global_set_event(EV_INIT_OK);
    }

    /* --- Super-loop principal --- */
    while (true) {
        /* 1. Servicio Modbus: se ejecuta solo si la ISR completo una trama */
        if (g_flag_modbus_rx_done) {
            g_flag_modbus_rx_done = false;
            modbus_service(&g_telemetry);
        }

        /* 2. Servicio temperatura + PWM (lazo proporcional, 100 Hz) */
        if (g_flag_pwm_tick) {
            g_flag_pwm_tick = false;
            float t_celsius = temp_sensor_read_celsius();
            g_telemetry.temp_internal = t_celsius;
            uint8_t duty = pwm_fan_compute_duty(t_celsius);
            pwm_fan_set_duty(duty);
            g_telemetry.pwm_duty = duty;
        }

        /* 3. Servicio OLED (5 Hz, refresco de pantalla) */
        if (g_flag_oled_refresh) {
            g_flag_oled_refresh = false;
            oled_service(&g_telemetry);
        }

        /* 4. Servicio HTTP (1 Hz, solo si estamos ONLINE) */
        if (g_flag_http_due && fsm_global_get_state() == STATE_ONLINE) {
            g_flag_http_due = false;
            http_post_telemetry(&g_telemetry);
        }

        /* 5. Servicio SD logger (solo si estamos OFFLINE_LOG) */
        if (g_flag_sd_write_pending &&
            fsm_global_get_state() == STATE_OFFLINE_LOG) {
            g_flag_sd_write_pending = false;
            sdcard_log_csv(&g_telemetry);
        }

        /* 6. Mantener stack lwIP / CYW43 */
        if (wifi_ok) {
            cyw43_arch_poll();
        }

        /* 7. Avanzar la FSM global */
        fsm_global_step(&g_telemetry);

        /* 8. WFE: dormir hasta el siguiente evento. Ahorra energia */
        tight_loop_contents();
    }

    return 0;  /* Nunca se alcanza */
}
