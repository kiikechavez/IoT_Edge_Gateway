/**
 * @file    temp_sensor.c
 * @brief   Implementacion del sensor de temperatura interno del RP2040
 * @author  Brando Enrique Chavez Vergara
 */
#include "temp_sensor.h"
#include "config.h"

#include "hardware/adc.h"
#include "pico/stdlib.h"

#define ADC_VREF        3.3f
#define ADC_RESOLUTION  4096.0f      /* ADC de 12 bits */

void temp_sensor_init(void) {
    adc_init();
    /* Habilita el sensor de temperatura interno del RP2040 */
    adc_set_temp_sensor_enabled(true);
    /* Selecciona el canal ADC4 (sensor termico interno) */
    adc_select_input(ADC_TEMP_CHANNEL);
}

float temp_sensor_read_celsius(void) {
    /* Asegura que estamos en el canal correcto (puede ser compartido) */
    adc_select_input(ADC_TEMP_CHANNEL);

    /* Promedio simple de 4 muestras para reducir ruido */
    uint32_t raw_sum = 0;
    for (int i = 0; i < 4; i++) {
        raw_sum += adc_read();
    }
    float raw_avg = (float)raw_sum / 4.0f;

    /* Conversion ADC -> voltaje */
    float v_adc = (raw_avg * ADC_VREF) / ADC_RESOLUTION;

    /* Formula de la hoja de datos del RP2040 (seccion 4.9.5) */
    float t_celsius = 27.0f - (v_adc - 0.706f) / 0.001721f;

    return t_celsius;
}
