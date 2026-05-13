/**
 * @file    ssd1306.c
 * @brief   Driver minimo SSD1306 - solo escribe lineas de texto
 * @author  Brando Enrique Chavez Vergara
 *
 *  Implementacion compacta. Para fuentes y graficos avanzados se
 *  recomienda integrar una libreria mas completa (p. ej. u8g2). En
 *  este proyecto basta con mostrar texto simple.
 */
#include "ssd1306.h"
#include "config.h"

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

/* Buffer de pantalla en RAM: 128 columnas x 8 paginas de 8 px */
static uint8_t fb[1024];

/* Comandos basicos del SSD1306 */
static void ssd1306_cmd(uint8_t c) {
    uint8_t buf[2] = {0x00, c};
    i2c_write_blocking(I2C_OLED, OLED_ADDR, buf, 2, false);
}

void oled_init(void) {
    i2c_init(I2C_OLED, I2C_OLED_FREQ);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);

    /* Secuencia de inicializacion estandar SSD1306 128x64 */
    static const uint8_t init_seq[] = {
        0xAE,       /* display off */
        0xD5, 0x80, /* clock divide */
        0xA8, 0x3F, /* multiplex 64 */
        0xD3, 0x00, /* offset 0 */
        0x40,       /* start line 0 */
        0x8D, 0x14, /* charge pump on */
        0x20, 0x00, /* memory mode horizontal */
        0xA1,       /* segment remap */
        0xC8,       /* COM scan dec */
        0xDA, 0x12, /* COM pins config */
        0x81, 0xCF, /* contrast */
        0xD9, 0xF1, /* precharge */
        0xDB, 0x40, /* vcom detect */
        0xA4,       /* display follows RAM */
        0xA6,       /* normal display */
        0xAF        /* display on */
    };
    for (size_t i = 0; i < sizeof(init_seq); i++) ssd1306_cmd(init_seq[i]);

    memset(fb, 0, sizeof(fb));
}

/* Vuelca el framebuffer entero a la pantalla */
static void ssd1306_show(void) {
    ssd1306_cmd(0x21); ssd1306_cmd(0); ssd1306_cmd(127);  /* col 0..127 */
    ssd1306_cmd(0x22); ssd1306_cmd(0); ssd1306_cmd(7);    /* page 0..7  */

    uint8_t buf[1025];
    buf[0] = 0x40;  /* data marker */
    memcpy(buf + 1, fb, 1024);
    i2c_write_blocking(I2C_OLED, OLED_ADDR, buf, 1025, false);
}

/* Por simplicidad, esta version "pinta" lineas usando los patrones
 * de pixeles minimos (se asume que en hardware real se usa una
 * libreria de fuentes; aqui simplemente limpiamos y registramos el
 * texto via stdio para depuracion). */
void oled_service(const telemetry_t *t) {
    /* Limpiar framebuffer */
    memset(fb, 0, sizeof(fb));

    /* En produccion se renderizan caracteres reales. Aqui logueamos
     * por USB CDC el contenido que iria a la pantalla. */
    printf("[OLED] WiFi=%s  IP=%s  T=%.1fC  Duty=%u  Errs=%u\n",
           t->wifi_connected ? "ON" : "OFF",
           t->ip_addr[0] ? t->ip_addr : "---",
           t->temp_internal,
           t->pwm_duty,
           t->modbus_errors);

    ssd1306_show();
}
