/**
 * @file    sdcard_logger.c
 * @brief   Datalogger CSV. Usa la libreria no-OS-FatFS-SD-SPI-RPi-Pico
 *          (https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico)
 *          que provee FatFs sobre SPI para el Pico.
 * @author  Brando Enrique Chavez Vergara
 */
#include "sdcard_logger.h"
#include "config.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>
#include <string.h>

/* Estos headers vienen de la libreria FatFs portada al Pico SDK.
 * Para que el codigo compile sin la libreria, se proveen stubs en
 * tiempo de prueba (vease sd_stub.c). */
#include "ff.h"
#include "sd_card.h"

static FATFS fs;
static FIL   file;
static bool  fs_mounted = false;

void sdcard_init(void) {
    /* La libreria sd_card.h gestiona la inicializacion SPI internamente
     * (lee pinout desde un archivo hw_config.c que debe proveerse).
     * Aqui solo intentamos montar el sistema de archivos. */
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("[sdcard] No se encontro tarjeta SD.\n");
        return;
    }
    FRESULT fr = f_mount(&fs, pSD->pcName, 1);
    if (fr != FR_OK) {
        printf("[sdcard] f_mount fallo: %d\n", fr);
        return;
    }
    fs_mounted = true;

    /* Si el CSV no existe, escribe cabecera */
    FILINFO fi;
    if (f_stat(SD_LOG_FILENAME, &fi) != FR_OK) {
        if (f_open(&file, SD_LOG_FILENAME, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
            f_printf(&file, "timestamp_ms,temp_c,pwm_duty,"
                            "reg0,reg1,reg2,reg3,reg4,reg5,reg6,reg7\n");
            f_close(&file);
        }
    }
    printf("[sdcard] Sistema de archivos montado OK.\n");
}

void sdcard_log_csv(const telemetry_t *t) {
    if (!fs_mounted) return;

    if (f_open(&file, SD_LOG_FILENAME, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
        printf("[sdcard] No se pudo abrir el log.\n");
        return;
    }

    f_printf(&file,
        "%lu,%.2f,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
        (unsigned long)t->modbus_timestamp_ms,
        (double)t->temp_internal,
        t->pwm_duty,
        t->modbus_regs[0], t->modbus_regs[1], t->modbus_regs[2],
        t->modbus_regs[3], t->modbus_regs[4], t->modbus_regs[5],
        t->modbus_regs[6], t->modbus_regs[7]
    );
    f_close(&file);
}

void sdcard_close(void) {
    if (fs_mounted) {
        sd_card_t *pSD = sd_get_by_num(0);
        if (pSD) f_unmount(pSD->pcName);
        fs_mounted = false;
    }
}
