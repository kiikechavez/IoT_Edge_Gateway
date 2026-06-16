/**
 * @file  ff_stub.c
 * @brief Implementaciones stub de FatFs y sd_card.
 *        Todas retornan FR_DISK_ERR / NULL para indicar que la SD no esta
 *        disponible. El sdcard_logger maneja esto con fs_mounted=false.
 *
 *        Reemplazar con la libreria real de carlk3 cuando se conecte la SD.
 */
#include "ff.h"
#include "sd_card.h"
#include <stdarg.h>

FRESULT f_mount(FATFS *fs, const char *path, BYTE opt)
    { (void)fs; (void)path; (void)opt; return FR_NOT_READY; }

FRESULT f_unmount(const char *path)
    { (void)path; return FR_OK; }

FRESULT f_open(FIL *fp, const char *path, BYTE mode)
    { (void)fp; (void)path; (void)mode; return FR_DISK_ERR; }

FRESULT f_close(FIL *fp)
    { (void)fp; return FR_OK; }

FRESULT f_stat(const char *path, FILINFO *fno)
    { (void)path; (void)fno; return FR_DISK_ERR; }

FRESULT f_sync(FIL *fp)
    { (void)fp; return FR_OK; }

int f_printf(FIL *fp, const char *fmt, ...)
    { (void)fp; (void)fmt; return -1; }

FRESULT f_write(FIL *fp, const void *buff, unsigned int btr, unsigned int *bw)
    { (void)fp; (void)buff; (void)btr; if (bw) *bw = 0; return FR_DISK_ERR; }

FRESULT f_read(FIL *fp, void *buff, unsigned int btr, unsigned int *br)
    { (void)fp; (void)buff; (void)btr; if (br) *br = 0; return FR_DISK_ERR; }

sd_card_t *sd_get_by_num(size_t num)
    { (void)num; return NULL; }
