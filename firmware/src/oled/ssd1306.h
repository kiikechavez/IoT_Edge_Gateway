/**
 * @file    ssd1306.h
 * @brief   Driver minimo SSD1306 (OLED 128x64) sobre I2C
 */
#ifndef SSD1306_H
#define SSD1306_H

#include "config.h"

/* Inicializa el bus I2C0 y el chip SSD1306 (secuencia de reset). */
void oled_init(void);

/* Servicio: refresca la pantalla con los datos de telemetria. */
void oled_service(const telemetry_t *t);

#endif /* SSD1306_H */
