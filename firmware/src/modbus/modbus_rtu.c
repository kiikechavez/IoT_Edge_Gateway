/**
 * @file    modbus_rtu.c
 * @brief   Maestro Modbus RTU con DMA + ISR + timer hardware
 * @author  Brando Enrique Chavez Vergara
 *
 *  Arquitectura:
 *    TX:  buffer en RAM --[DMA0]--> UART0_TX --[MAX485]--> bus RS-485
 *    RX:  bus RS-485 --[MAX485]--> UART0_RX --[DMA1]--> buffer en RAM
 *
 *  Manejo de tiempos (RNF2):
 *    Tras cada byte recibido (ISR UART0_IRQ), se reinicia un timer
 *    hardware programado a 3.5 caracteres (~4 ms a 9600 baud).
 *    Si el timer expira sin nuevos bytes, la trama esta completa.
 */
#include "modbus_rtu.h"
#include "config.h"

#include "hardware/uart.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

#define MODBUS_BUF_LEN     64

/* Buffers TX/RX accedidos por DMA */
static uint8_t  tx_buf[MODBUS_BUF_LEN];
static uint8_t  rx_buf[MODBUS_BUF_LEN];
static uint     dma_tx_chan;
static uint     dma_rx_chan;
static volatile uint16_t rx_index = 0;
static alarm_id_t silence_alarm = 0;

/* ----------------------------------------------------------------------------
 *  Calculo del CRC16 Modbus (polinomio 0xA001, reflexion seed 0xFFFF)
 * --------------------------------------------------------------------------*/
static uint16_t modbus_crc16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else              crc >>= 1;
        }
    }
    return crc;
}

/* ----------------------------------------------------------------------------
 *  Callback del timer 3.5char: la trama termino
 * --------------------------------------------------------------------------*/
static int64_t on_silence_timeout(alarm_id_t id, void *user_data) {
    (void)id; (void)user_data;
    g_flag_modbus_rx_done = true;
    return 0; /* no reprogramar */
}

/* ----------------------------------------------------------------------------
 *  ISR del UART0: por cada byte recibido, lo guarda y reinicia el timer
 * --------------------------------------------------------------------------*/
static void on_uart0_rx(void) {
    while (uart_is_readable(UART_MODBUS) && rx_index < MODBUS_BUF_LEN) {
        rx_buf[rx_index++] = uart_getc(UART_MODBUS);
    }
    /* Reprograma el timer de silencio */
    if (silence_alarm) cancel_alarm(silence_alarm);
    silence_alarm = add_alarm_in_us(MODBUS_INTERFRAME_US,
                                    on_silence_timeout, NULL, false);
}

/* ----------------------------------------------------------------------------
 *  Inicializacion
 * --------------------------------------------------------------------------*/
void modbus_init(void) {
    /* UART0 a 9600 8N1 */
    uart_init(UART_MODBUS, UART_MODBUS_BAUD);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
    uart_set_format(UART_MODBUS, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_MODBUS, false);

    /* Pin DE/RE_n del MAX485 */
    gpio_init(PIN_MAX485_DE_RE);
    gpio_set_dir(PIN_MAX485_DE_RE, GPIO_OUT);
    gpio_put(PIN_MAX485_DE_RE, 0);   /* Empieza en modo recepcion */

    /* Reserva 2 canales DMA */
    dma_tx_chan = dma_claim_unused_channel(true);
    dma_rx_chan = dma_claim_unused_channel(true);

    /* ISR del UART0 RX */
    irq_set_exclusive_handler(UART0_IRQ, on_uart0_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_MODBUS, true, false);

    printf("[modbus] Inicializado. DMA TX=%u, DMA RX=%u\n",
           dma_tx_chan, dma_rx_chan);
}

/* ----------------------------------------------------------------------------
 *  Envio de peticion FC03 con DMA TX
 * --------------------------------------------------------------------------*/
void modbus_request_holding_regs(void) {
    /* Construye la trama: addr | FC | reg_h | reg_l | qty_h | qty_l | CRC_l | CRC_h */
    tx_buf[0] = MODBUS_SLAVE_ADDR;
    tx_buf[1] = MODBUS_FC_READ_HOLDING;
    tx_buf[2] = (MODBUS_START_REG >> 8) & 0xFF;
    tx_buf[3] =  MODBUS_START_REG       & 0xFF;
    tx_buf[4] = (MODBUS_REG_COUNT >> 8) & 0xFF;
    tx_buf[5] =  MODBUS_REG_COUNT       & 0xFF;
    uint16_t crc = modbus_crc16(tx_buf, 6);
    tx_buf[6] = crc & 0xFF;
    tx_buf[7] = (crc >> 8) & 0xFF;

    /* Reinicia el buffer de recepcion */
    rx_index = 0;
    memset(rx_buf, 0, sizeof(rx_buf));

    /* Habilita transmision en el MAX485 */
    gpio_put(PIN_MAX485_DE_RE, 1);
    sleep_us(50);   /* turn-around time */

    /* Configura DMA0 -> UART0_TX */
    dma_channel_config c = dma_channel_get_default_config(dma_tx_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, DREQ_UART0_TX);
    dma_channel_configure(dma_tx_chan, &c,
                          &uart_get_hw(UART_MODBUS)->dr,  /* destino */
                          tx_buf,                         /* origen  */
                          8,                              /* len     */
                          true);                          /* start   */

    /* Espera fin de la transmision */
    dma_channel_wait_for_finish_blocking(dma_tx_chan);
    /* Espera vaciado del shift register del UART */
    uart_tx_wait_blocking(UART_MODBUS);

    /* Cambia a recepcion */
    sleep_us(50);
    gpio_put(PIN_MAX485_DE_RE, 0);
}

/* ----------------------------------------------------------------------------
 *  Procesa la respuesta (CRC + extraccion de registros)
 * --------------------------------------------------------------------------*/
bool modbus_service(telemetry_t *t) {
    if (rx_index < 5) {
        /* Trama demasiado corta */
        t->modbus_errors++;
        return false;
    }

    /* Verifica CRC */
    uint16_t crc_calc = modbus_crc16(rx_buf, rx_index - 2);
    uint16_t crc_recv = rx_buf[rx_index - 2] |
                       (rx_buf[rx_index - 1] << 8);
    if (crc_calc != crc_recv) {
        t->modbus_errors++;
        return false;
    }

    /* Verifica direccion y FC */
    if (rx_buf[0] != MODBUS_SLAVE_ADDR ||
        rx_buf[1] != MODBUS_FC_READ_HOLDING) {
        t->modbus_errors++;
        return false;
    }

    /* Extrae los registros (byte_count en rx_buf[2]) */
    uint8_t byte_count = rx_buf[2];
    uint16_t n_regs = byte_count / 2;
    if (n_regs > MODBUS_REG_COUNT) n_regs = MODBUS_REG_COUNT;
    for (uint16_t i = 0; i < n_regs; i++) {
        t->modbus_regs[i] = (rx_buf[3 + i*2] << 8) | rx_buf[4 + i*2];
    }
    t->modbus_timestamp_ms = to_ms_since_boot(get_absolute_time());
    t->total_samples++;
    return true;
}
