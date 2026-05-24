/**
 * @file    sdcard_logger.h
 * @brief   Datalogger CSV sobre microSD (SPI + FatFs)
 */
#ifndef SDCARD_LOGGER_H
#define SDCARD_LOGGER_H

#include "config.h"

/* Inicializa SPI1 + monta el sistema de archivos FAT32. */
void sdcard_init(void);

/* Escribe una fila CSV con la telemetria actual. */
void sdcard_log_csv(const telemetry_t *t);

/* Cierra archivos y desmonta el sistema de archivos. */
void sdcard_close(void);

#endif /* SDCARD_LOGGER_H */
