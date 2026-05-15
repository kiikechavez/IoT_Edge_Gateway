/*
 * Modbus RTU master - WIP: solo CRC16 y armado de trama por ahora.
 * DMA, ISR y timer 3.5 char vendran en commits siguientes.
 */
#include "modbus_rtu.h"

static uint16_t modbus_crc16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else         crc >>= 1;
        }
    }
    return crc;
}

void modbus_init(void) { /* TODO */ }
void modbus_request_holding_regs(void) { /* TODO */ }
bool modbus_service(telemetry_t *t) { (void)t; return false; }
