/**
 * @file    fsm_global.c
 * @brief   Implementacion de la FSM global (INIT/ONLINE/OFFLINE_LOG/ERROR)
 * @author  Brando Enrique Chavez Vergara
 */
#include "fsm_global.h"
#include "config.h"
#include "wifi/wifi_manager.h"

#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include <stdio.h>

static fsm_state_t cur_state;
static volatile fsm_event_t pending_event;
static uint8_t   modbus_timeout_count;

void fsm_global_init(void) {
    cur_state            = STATE_INIT;
    pending_event        = EV_NONE;
    modbus_timeout_count = 0;
}

fsm_state_t fsm_global_get_state(void) {
    return cur_state;
}

void fsm_global_set_event(fsm_event_t ev) {
    pending_event = ev;
}

const char *fsm_global_state_name(fsm_state_t s) {
    switch (s) {
        case STATE_INIT:        return "INIT";
        case STATE_ONLINE:      return "ONLINE";
        case STATE_OFFLINE_LOG: return "OFFLINE_LOG";
        case STATE_ERROR:       return "ERROR";
        default:                return "?";
    }
}

/* ----------------------------------------------------------------------------
 *  Avanza un paso de la FSM. Llamada desde el super-loop.
 * --------------------------------------------------------------------------*/
void fsm_global_step(telemetry_t *t) {
    /* Verificar enlace Wi-Fi cada paso */
    bool wifi_up = wifi_manager_is_connected();
    if (!wifi_up && cur_state == STATE_ONLINE) {
        pending_event = EV_WIFI_LOST;
    } else if (wifi_up && cur_state == STATE_OFFLINE_LOG) {
        pending_event = EV_WIFI_RECONNECTED;
    }
    t->wifi_connected = wifi_up;

    /* Verificar errores acumulados de Modbus */
    if (t->modbus_errors > 0 && t->modbus_errors % 3 == 0
        && cur_state == STATE_ONLINE) {
        pending_event = EV_MODBUS_TIMEOUT;
    }

    /* Transiciones */
    fsm_event_t ev = pending_event;
    pending_event = EV_NONE;
    fsm_state_t prev = cur_state;

    switch (cur_state) {
        case STATE_INIT:
            if (ev == EV_INIT_OK)            cur_state = STATE_ONLINE;
            else if (ev == EV_INIT_FAIL_WIFI) cur_state = STATE_OFFLINE_LOG;
            break;

        case STATE_ONLINE:
            if (ev == EV_WIFI_LOST)           cur_state = STATE_OFFLINE_LOG;
            else if (ev == EV_MODBUS_TIMEOUT) cur_state = STATE_ERROR;
            break;

        case STATE_OFFLINE_LOG:
            if (ev == EV_WIFI_RECONNECTED)    cur_state = STATE_ONLINE;
            else if (ev == EV_SD_FAIL)        cur_state = STATE_ERROR;
            break;

        case STATE_ERROR:
            /* Espera el watchdog (configurado para 10s) */
            if (ev == EV_WATCHDOG) {
                watchdog_enable(1, true);
                while (1) { tight_loop_contents(); }
            }
            break;
    }

    /* En estado OFFLINE_LOG habilitamos la escritura de la SD */
    g_flag_sd_write_pending = (cur_state == STATE_OFFLINE_LOG);

    if (prev != cur_state) {
        printf("[fsm] %s -> %s\n",
               fsm_global_state_name(prev),
               fsm_global_state_name(cur_state));
    }
}
