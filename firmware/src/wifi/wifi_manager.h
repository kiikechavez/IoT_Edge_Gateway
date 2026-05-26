/**
 * @file    wifi_manager.h
 * @brief   Gestor de la conexion Wi-Fi (CYW43439 del Pico W)
 */
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include "config.h"

/**
 * Inicializa el chip CYW43439, intenta conectarse a la SSID configurada
 * en config.h, y obtiene una IP via DHCP.
 * Llena g_telemetry.wifi_connected y g_telemetry.ip_addr.
 *
 * @return  true si la conexion fue exitosa.
 */
bool wifi_manager_init(void);

/**
 * Verifica el estado del enlace Wi-Fi. Retorna true si sigue conectado.
 * Usa cyw43_wifi_link_status() (cumple RNF1).
 */
bool wifi_manager_is_connected(void);

/* Llena t con la IP actual y el flag wifi_connected. */
void wifi_manager_update_status(telemetry_t *t);

#endif /* WIFI_MANAGER_H */
