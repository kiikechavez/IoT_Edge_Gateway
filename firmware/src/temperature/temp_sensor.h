/**
 * @file    temp_sensor.h
 * @brief   Lectura del sensor de temperatura interno del RP2040 (ADC4)
 */
#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

/**
 * Inicializa el ADC y selecciona el canal interno (ADC4) que esta
 * conectado al sensor de temperatura del RP2040.
 */
void temp_sensor_init(void);

/**
 * Lee una muestra del ADC4 y la convierte a grados Celsius usando
 * la formula de la hoja de datos del RP2040 (seccion 4.9.5).
 *
 *   T[C] = 27 - (V_adc - 0.706) / 0.001721
 *
 * @return  Temperatura en grados Celsius (float)
 */
float temp_sensor_read_celsius(void);

#endif /* TEMP_SENSOR_H */
