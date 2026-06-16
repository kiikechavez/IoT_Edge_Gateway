/**
 * @file    config.h
 * @brief   Configuracion global del proyecto IoT Edge Gateway
 * @author  Brando Enrique Chavez Vergara
 *
 * Contiene:
 *  - Pinout completo (GPIOs del Pico W -> perifericos)
 *  - Constantes de protocolos (baudrate, frecuencias, timeouts)
 *  - Direcciones de red y endpoints
 *  - Estructura de telemetria compartida entre modulos
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>


/* --- UART0 - Modbus RTU (a traves de MAX485) --- */
#define UART_MODBUS              uart0
#define UART_MODBUS_BAUD         9600       /* Baud rate Modbus RTU         */
#define PIN_UART_TX              0          /* GP0  -> DI  del MAX485       */
#define PIN_UART_RX              1          /* GP1  <- RO  del MAX485       */
#define PIN_MAX485_DE_RE         3          /* GP3  -> DE/RE_n del MAX485   */

/* --- I2C0 - Pantalla OLED SSD1306 --- */
#define I2C_OLED                 i2c0
#define I2C_OLED_FREQ            400000     /* 400 kHz fast mode            */
#define PIN_I2C_SDA              4          /* GP4 -> SDA OLED              */
#define PIN_I2C_SCL              5          /* GP5 -> SCL OLED              */
#define OLED_ADDR                0x3C       /* I2C address por defecto      */

/* --- SPI1 - Lector microSD --- */
#define SPI_SD                   spi1
#define SPI_SD_FREQ              12500000   /* 12.5 MHz                     */
#define PIN_SPI_SCK              10         /* GP10 -> SCK SD               */
#define PIN_SPI_MOSI             11         /* GP11 -> MOSI SD              */
#define PIN_SPI_MISO             12         /* GP12 <- MISO SD              */
#define PIN_SPI_CS               13         /* GP13 -> CS SD                */

/* --- PWM - Ventilador DC --- */
#define PIN_PWM_FAN              15         /* GP15 -> Gate IRLZ44N         */
#define PWM_FAN_FREQ_HZ          25000      /* 25 kHz (fuera del audible)   */
#define PWM_FAN_WRAP             255        /* 8 bits de resolucion         */

/* --- ADC - Sensor de temperatura interno del RP2040 --- */
#define ADC_TEMP_CHANNEL         4          /* Canal interno ADC4           */

//PARAMETROS DEL CONTROL TERMICO
#define TEMP_THRESHOLD_C         20.0f      /* Por debajo: ventilador OFF   */
#define TEMP_MAX_C               60.0f      /* En este punto: duty = 100%   */
#define CONTROL_LOOP_HZ          100        /* Frecuencia del lazo cerrado  */


//MODBUS RTU

#define MODBUS_SLAVE_ADDR        0x01       /* Direccion del esclavo (planta)*/
#define MODBUS_FC_READ_HOLDING   0x03       /* Function code: Read Holding  */
#define MODBUS_START_REG         0x0000     /* Direccion inicial            */
#define MODBUS_REG_COUNT         8          /* Numero de holding registers  */
#define MODBUS_TIMEOUT_MS        100        /* Timeout total                */
#define MODBUS_INTERFRAME_US     4010       /* 3.5 char @ 9600 = 4.01 ms    */
#define MODBUS_POLL_PERIOD_MS    300        /* Periodo entre peticiones FC03*/


//WI-FI / HTTP

#define WIFI_SSID                "iPhone kiki"
#define WIFI_PASSWORD            "momentokiki"
#define HTTP_SERVER_IP           "172.20.10.9"       /* IP del servidor   */
#define HTTP_SERVER_PORT         8000
#define HTTP_ENDPOINT            "/api/telemetry/"
#define HTTP_POST_PERIOD_MS      1000                  /* 1 POST/segundo    */


// SD CARD

#define SD_LOG_FILENAME          "telemetry.csv"


// ESTRUCTURA DE TELEMETRIA GLOBAL
// Compartida entre todos los modulos. Una sola instancia: g_telemetry.

typedef struct {
    /* --- Variables locales (medidas en el Pico) --- */
    float    temp_internal;          /* Temperatura ADC4 en grados C        */
    uint8_t  pwm_duty;               /* Duty cycle PWM 0..255               */

    /* --- Variables industriales (leidas del esclavo Modbus) --- */
    uint16_t modbus_regs[MODBUS_REG_COUNT];   /* Holding Registers          */
    uint32_t modbus_timestamp_ms;             /* Cuando se leyeron          */
    uint16_t modbus_errors;                   /* Contador de timeouts       */

    /* --- Estado de red --- */
    bool     wifi_connected;
    char     ip_addr[16];

    /* --- Contadores --- */
    uint32_t total_samples;
    uint32_t total_uploads;
    uint32_t total_offline_writes;
} telemetry_t;

extern telemetry_t g_telemetry;

/* Flags globales (modificadas por ISRs) */
extern volatile bool g_flag_modbus_rx_done;
extern volatile bool g_flag_pwm_tick;
extern volatile bool g_flag_oled_refresh;
extern volatile bool g_flag_http_due;
extern volatile bool g_flag_sd_write_pending;

#endif /* CONFIG_H */
