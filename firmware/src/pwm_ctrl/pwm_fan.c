/**
 * @file    pwm_fan.c
 * @brief   Implementacion del lazo proporcional de control de ventilador
 * @author  Brando Enrique Chavez Vergara
 *
 * Uso del Pico SDK:
 *   - hardware/pwm.h : canal PWM 7B en GP15
 *   - hardware/irq.h : ISR de PWM_WRAP para tick del lazo (100 Hz)
 */
#include "pwm_fan.h"
#include "config.h"

#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"

static uint slice_num;

/**
 * ISR del wrap de PWM (cuenta hasta WRAP y se reinicia).
 * Como el wrap ocurre a 25 kHz, dividimos por 250 para obtener 100 Hz.
 */
static volatile uint16_t wrap_counter = 0;

static void on_pwm_wrap(void) {
    pwm_clear_irq(slice_num);
    wrap_counter++;
    if (wrap_counter >= 250) {
        wrap_counter = 0;
        g_flag_pwm_tick = true;          /* Avisa al super-loop */
    }
}

void pwm_fan_init(void) {
    /* Selecciona la funcion PWM en GP15 */
    gpio_set_function(PIN_PWM_FAN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(PIN_PWM_FAN);

    /* Configura frecuencia ~25 kHz con clock divider */
    pwm_config cfg = pwm_get_default_config();
    /* clk_sys es ~125 MHz; 125e6 / (255 * 25e3) ~= 19.6 */
    pwm_config_set_clkdiv(&cfg, 19.6f);
    pwm_config_set_wrap(&cfg, PWM_FAN_WRAP);
    pwm_init(slice_num, &cfg, true);

    /* Arranca con duty=0 (ventilador apagado) */
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PIN_PWM_FAN), 0);

    /* Habilita ISR de wrap para el tick del lazo */
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
}

void pwm_fan_set_duty(uint8_t duty) {
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PIN_PWM_FAN), duty);
}

uint8_t pwm_fan_compute_duty(float t_celsius) {
    if (t_celsius < TEMP_THRESHOLD_C) {
        return 0;     /* Apagado: por debajo del umbral */
    }
    if (t_celsius >= TEMP_MAX_C) {
        return 255;   /* Maximo: por encima del techo */
    }
    /* Interpolacion lineal entre THRESHOLD y MAX */
    float range = TEMP_MAX_C - TEMP_THRESHOLD_C;
    float norm  = (t_celsius - TEMP_THRESHOLD_C) / range;  /* 0..1 */
    return (uint8_t)(norm * 255.0f);
}
