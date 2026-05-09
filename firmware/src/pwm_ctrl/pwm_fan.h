/**
 * @file    pwm_fan.h
 * @brief   Control PWM del ventilador DC (lazo proporcional)
 */
#ifndef PWM_FAN_H
#define PWM_FAN_H

#include <stdint.h>

/**
 * Configura el canal PWM del GP15 a 25 kHz con 8 bits de resolucion.
 * Tambien habilita la interrupcion PWM_WRAP a 100 Hz para el tick del
 * lazo de control.
 */
void pwm_fan_init(void);

/**
 * Establece el duty cycle (0-255 -> 0-100 %).
 */
void pwm_fan_set_duty(uint8_t duty);

/**
 * Lazo de control proporcional. Si T < THRESHOLD -> duty=0.
 * Si T > MAX -> duty=255. Lineal entre ambos.
 *
 * @param  t_celsius   Temperatura actual en grados Celsius
 * @return  duty 0..255
 */
uint8_t pwm_fan_compute_duty(float t_celsius);

#endif /* PWM_FAN_H */
