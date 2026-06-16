/**
 * @file  sd_card.h
 * @brief Stub de la interfaz sd_card de no-OS-FatFS-SD-SPI-RPi-Pico.
 *        Retorna NULL en sd_get_by_num() para indicar que no hay SD disponible.
 */
#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdint.h>

typedef struct {
    const char *pcName;   /* nombre del volumen, e.g. "0:" */
    uint8_t     dummy;
} sd_card_t;

/* Devuelve NULL — sin tarjeta SD en este stub */
sd_card_t *sd_get_by_num(size_t num);

#endif /* SD_CARD_H */
