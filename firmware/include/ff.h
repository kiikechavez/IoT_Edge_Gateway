/**
 * @file  ff.h
 * @brief Stub minimo de FatFs para compilar sin la libreria
 *        no-OS-FatFS-SD-SPI-RPi-Pico.
 *
 *        Todas las funciones retornan FR_DISK_ERR para indicar que la SD
 *        no esta disponible. El modulo sdcard_logger maneja este error
 *        de forma graciosa (fs_mounted queda en false y no escribe).
 *
 *        Para habilitar SD real: reemplazar este archivo y ff.c con la
 *        libreria de carlk3 (https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico).
 */
#ifndef FF_H
#define FF_H

#include <stdint.h>
#include <stddef.h>

/* --- Tipos basicos de FatFs --- */
typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef DWORD    FSIZE_t;

typedef struct { BYTE dummy; } FATFS;   /* objeto de sistema de archivos    */
typedef struct { BYTE dummy; } FIL;     /* objeto de archivo                */
typedef struct {
    FSIZE_t fsize;
    WORD    fdate;
    WORD    ftime;
    BYTE    fattrib;
    char    fname[256];
} FILINFO;

/* --- Codigos de resultado --- */
typedef enum {
    FR_OK = 0,
    FR_DISK_ERR,
    FR_INT_ERR,
    FR_NOT_READY,
    FR_NO_FILE,
    FR_NO_PATH,
    FR_INVALID_NAME,
    FR_DENIED,
    FR_EXIST,
    FR_INVALID_OBJECT,
    FR_WRITE_PROTECTED,
    FR_INVALID_DRIVE,
    FR_NOT_ENABLED,
    FR_NO_FILESYSTEM,
    FR_MKFS_ABORTED,
    FR_TIMEOUT,
    FR_LOCKED,
    FR_NOT_ENOUGH_CORE,
    FR_TOO_MANY_OPEN_FILES,
    FR_INVALID_PARAMETER
} FRESULT;

/* --- Flags de apertura --- */
#define FA_READ             0x01
#define FA_WRITE            0x02
#define FA_OPEN_EXISTING    0x00
#define FA_CREATE_NEW       0x04
#define FA_CREATE_ALWAYS    0x08
#define FA_OPEN_ALWAYS      0x10
#define FA_OPEN_APPEND      0x30

/* --- Declaraciones de funciones (implementadas en ff.c stub) --- */
FRESULT f_mount(FATFS *fs, const char *path, BYTE opt);
FRESULT f_unmount(const char *path);
FRESULT f_open(FIL *fp, const char *path, BYTE mode);
FRESULT f_close(FIL *fp);
FRESULT f_stat(const char *path, FILINFO *fno);
FRESULT f_sync(FIL *fp);
int     f_printf(FIL *fp, const char *fmt, ...);
FRESULT f_write(FIL *fp, const void *buff, unsigned int btr, unsigned int *bw);
FRESULT f_read(FIL *fp, void *buff, unsigned int btr, unsigned int *br);

#endif /* FF_H */
