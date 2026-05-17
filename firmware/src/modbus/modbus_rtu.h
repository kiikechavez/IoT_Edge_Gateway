/**
 * @file    modbus_rtu.h
 * @brief   Maestro Modbus RTU sobre UART0 + RS-485 (MAX485)
 *
 *  - Usa DMA para TX y RX (cumple RNF2 -- precision de tiempos).
 *  - Usa un timer hardware para detectar el silencio inter-frame
 *    de 3.5 caracteres (~4 ms a 9600 baud).
 *  - Patron polling + interrupciones.
 */
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "config.h"

/**
 * Inicializa UART0, los dos canales DMA (TX y RX) y el timer 3.5char.
 * Configura el pin DE/RE_n del MAX485 como salida.
 */
void modbus_init(void);

/**
 * Envia una peticion Modbus FC03 (Read Holding Registers) al esclavo
 * configurado y arma la recepcion DMA. Retorna inmediatamente.
 * Cuando la respuesta llega, la ISR levanta g_flag_modbus_rx_done.
 */
void modbus_request_holding_regs(void);

/**
 * Procesa la respuesta pendiente en el buffer (valida CRC, llena la
 * estructura de telemetria). Se llama desde el super-loop cuando
 * g_flag_modbus_rx_done == true.
 *
 * @return  true si la respuesta fue valida y los registros se
 *          actualizaron en t->modbus_regs[]; false en caso contrario.
 */
bool modbus_service(telemetry_t *t);

#endif /* MODBUS_RTU_H */
