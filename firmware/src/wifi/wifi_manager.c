/**
 * @file    wifi_manager.c
 * @brief   Inicializacion y monitoreo del CYW43439 + lwIP
 * @author  Brando Enrique Chavez Vergara
 */
#include "wifi_manager.h"
#include "config.h"

#include "pico/cyw43_arch.h"
#include "lwip/ip4_addr.h"
#include <stdio.h>
#include <string.h>

bool wifi_manager_init(void) {
    if (cyw43_arch_init()) {
        printf("[wifi] cyw43_arch_init() fallo.\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();

    printf("[wifi] Conectando a SSID '%s' ...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                          CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("[wifi] No se pudo conectar.\n");
        return false;
    }

    /* Obtener IP */
    struct netif *nif = netif_default;
    if (nif) {
        snprintf(g_telemetry.ip_addr, sizeof(g_telemetry.ip_addr),
                 "%s", ip4addr_ntoa(netif_ip4_addr(nif)));
    }
    g_telemetry.wifi_connected = true;
    printf("[wifi] Conectado. IP = %s\n", g_telemetry.ip_addr);
    return true;
}

bool wifi_manager_is_connected(void) {
    int s = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    return (s == CYW43_LINK_UP);
}

void wifi_manager_update_status(telemetry_t *t) {
    t->wifi_connected = wifi_manager_is_connected();
    if (!t->wifi_connected) t->ip_addr[0] = '\0';
}
