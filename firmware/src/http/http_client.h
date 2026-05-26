/**
 * @file    http_client.h
 * @brief   Cliente HTTP POST con serializacion JSON (cumple RNF3)
 */
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stdbool.h>
#include "config.h"

/**
 * Envia un POST HTTP a HTTP_SERVER_IP:HTTP_SERVER_PORT/HTTP_ENDPOINT
 * con un cuerpo JSON construido a partir de la telemetria.
 *
 *  Ejemplo del JSON enviado:
 *  {
 *    "ts": 12345,
 *    "temp": 32.5,
 *    "duty": 64,
 *    "regs": [10, 20, 30, 40, 50, 60, 70, 80],
 *    "errors": 0
 *  }
 *
 * @return  true si el POST fue aceptado (HTTP 2xx).
 */
bool http_post_telemetry(const telemetry_t *t);

#endif /* HTTP_CLIENT_H */
