/**
 * @file    fsm_global.h
 * @brief   Maquina de estados global del sistema
 *
 *  Estados:  INIT, ONLINE, OFFLINE_LOG, ERROR
 *  Documentada en el PDF de arquitectura (pregunta 5).
 */
#ifndef FSM_GLOBAL_H
#define FSM_GLOBAL_H

#include "config.h"

typedef enum {
    STATE_INIT,
    STATE_ONLINE,
    STATE_OFFLINE_LOG,
    STATE_ERROR
} fsm_state_t;

typedef enum {
    EV_NONE,
    EV_INIT_OK,
    EV_INIT_FAIL_WIFI,
    EV_WIFI_LOST,
    EV_WIFI_RECONNECTED,
    EV_MODBUS_TIMEOUT,
    EV_SD_FAIL,
    EV_WATCHDOG
} fsm_event_t;

void        fsm_global_init(void);
fsm_state_t fsm_global_get_state(void);
void        fsm_global_set_event(fsm_event_t ev);
void        fsm_global_step(telemetry_t *t);
const char *fsm_global_state_name(fsm_state_t s);

#endif /* FSM_GLOBAL_H */
