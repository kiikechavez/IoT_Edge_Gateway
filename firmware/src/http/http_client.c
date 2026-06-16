/**
 * @file    http_client.c
 * @brief   Cliente HTTP POST minimalista usando sockets crudos de lwIP
 * @author  Brando Enrique Chavez Vergara
 *
 *  Construye el JSON con snprintf() (sin librerias externas) y abre
 *  un socket TCP directo al backend. La cabecera HTTP es minima.
 */
#include "http_client.h"
#include "config.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/ip4_addr.h"
#include <string.h>
#include <stdio.h>

#define JSON_BUF_LEN   512
#define HTTP_BUF_LEN   1024

/* Estado de la conexion TCP (lwIP es no-bloqueante) */
typedef struct {
    struct tcp_pcb *pcb;
    char            http_buf[HTTP_BUF_LEN];
    uint16_t        http_len;
    bool            response_ok;
    bool            done;
} http_state_t;

/* ----------------------------------------------------------------------------
 *  Construye el cuerpo JSON
 * --------------------------------------------------------------------------*/
static int build_json(char *buf, size_t buflen, const telemetry_t *t) {
    return snprintf(buf, buflen,
        "{"
          "\"ts\":%lu,"
          "\"temp\":%.2f,"
          "\"duty\":%u,"
          "\"regs\":[%u,%u,%u,%u,%u,%u,%u,%u],"
          "\"errors\":%u"
        "}",
        (unsigned long)t->modbus_timestamp_ms,
        (double)t->temp_internal,
        t->pwm_duty,
        t->modbus_regs[0], t->modbus_regs[1], t->modbus_regs[2],
        t->modbus_regs[3], t->modbus_regs[4], t->modbus_regs[5],
        t->modbus_regs[6], t->modbus_regs[7],
        t->modbus_errors);
}

/* ----------------------------------------------------------------------------
 *  Callback de recepcion de la respuesta del servidor
 * --------------------------------------------------------------------------*/
static err_t http_recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
                          err_t err) {
    http_state_t *st = (http_state_t*)arg;
    if (!p) {
        st->done = true;
        tcp_close(pcb);
        return ERR_OK;
    }
    /* Buscar "HTTP/1.1 2xx" en los primeros bytes */
    if (p->len > 12) {
        char *resp = (char*)p->payload;
        if (resp[9] == '2') st->response_ok = true;
    }
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

/* ----------------------------------------------------------------------------
 *  Envio del POST
 * --------------------------------------------------------------------------*/
bool http_post_telemetry(const telemetry_t *t) {
    char json[JSON_BUF_LEN];
    int json_len = build_json(json, sizeof(json), t);
    if (json_len < 0) return false;

    static http_state_t state;
    memset(&state, 0, sizeof(state));

    state.http_len = snprintf(state.http_buf, HTTP_BUF_LEN,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        HTTP_ENDPOINT, HTTP_SERVER_IP, json_len, json);

    /* Crear PCB TCP */
    state.pcb = tcp_new();
    if (!state.pcb) return false;

    ip4_addr_t srv_ip;
    ip4addr_aton(HTTP_SERVER_IP, &srv_ip);

    tcp_arg(state.pcb, &state);
    tcp_recv(state.pcb, http_recv_cb);

    err_t err = tcp_connect(state.pcb, &srv_ip, HTTP_SERVER_PORT, NULL);
    if (err != ERR_OK) {
        tcp_close(state.pcb);
        return false;
    }

    /* Enviar la peticion */
    tcp_write(state.pcb, state.http_buf, state.http_len, TCP_WRITE_FLAG_COPY);
    tcp_output(state.pcb);

    /* Esperar respuesta no-bloqueante (max 2 s).
     * cyw43_arch_poll() es obligatorio para que lwIP procese los paquetes
     * en modo polling (sin RTOS). Sin esta llamada el TCP nunca avanza. */
    uint32_t t0 = to_ms_since_boot(get_absolute_time());
    while (!state.done && (to_ms_since_boot(get_absolute_time()) - t0) < 2000) {
        cyw43_arch_poll();
        sleep_ms(1);
    }
    return state.response_ok;
}
