# Esquemático y Guía de Conexiones — IoT Edge Gateway

Documento de referencia con todas las conexiones físicas necesarias para
montar el prototipo. Sigue este documento al armar la protoboard.

---

## 1. Pinout completo de la Raspberry Pi Pico W

```
                    Raspberry Pi Pico W
                    +-----------------+
              GP0  -| 1  TX     VBUS  |- 40  --> +5V (alimentacion)
              GP1  -| 2  RX     VSYS  |- 39  --> +5V (in)
              GND  -| 3                |- 38  --> GND
              GP2  -| 4         3V3_EN|- 37
              GP3  -| 5  DE/RE  3V3   |- 36  --> +3.3V (modulos logicos)
              GP4  -| 6  SDA    AREF  |- 35
              GP5  -| 7  SCL    ADC2  |- 34
              GND  -| 8                |- 33  --> GND
              GP6  -| 9          ADC1  |- 32
              GP7  -| 10         ADC0  |- 31
              GP8  -| 11        RUN   |- 30
              GP9  -| 12        GP22  |- 29
              GND  -| 13                |- 28  --> GND
              GP10 -| 14 SCK    GP21  |- 27
              GP11 -| 15 MOSI   GP20  |- 26
              GP12 -| 16 MISO   GP19  |- 25
              GP13 -| 17 CS     GP18  |- 24
              GND  -| 18                |- 23  --> GND
              GP14 -| 19        GP17  |- 22
              GP15 -| 20 PWM    GP16  |- 21
                    +-----------------+
```

> Nota: el sensor de temperatura interno está en el canal ADC4
> (no tiene pin físico — es interno al RP2040).

---

## 2. Conexión del transceptor RS-485 (MAX485)

| Pin MAX485 | Conectar a              | Notas                                  |
|:----------:|-------------------------|----------------------------------------|
| RO         | GP1 (UART0 RX) del Pico | Salida del receptor                    |
| RE         | GP3 del Pico (junto a DE)| Receiver enable (activo en bajo)       |
| DE         | GP3 del Pico (junto a RE)| Driver enable (activo en alto)         |
| DI         | GP0 (UART0 TX) del Pico | Entrada del driver                     |
| VCC        | +5V                     | Alimentación                           |
| GND        | GND                     | Tierra común                           |
| A          | Línea A del bus RS-485  | Conector hacia la planta Modbus        |
| B          | Línea B del bus RS-485  | Conector hacia la planta Modbus        |

### Acondicionamiento del bus

- Resistencia de **120 Ω** entre A y B en cada extremo del bus (terminación).
- Resistencia de **680 Ω** entre B y +5V (pull-up fail-safe).
- Resistencia de **680 Ω** entre A y GND (pull-down fail-safe).

---

## 3. Conexión de la pantalla OLED 0.96" (SSD1306)

| Pin OLED | Conectar a    |
|:--------:|---------------|
| VCC      | +3.3V         |
| GND      | GND           |
| SCL      | GP5 (I2C0 SCL)|
| SDA      | GP4 (I2C0 SDA)|

> Las resistencias pull-up de I2C ya vienen integradas en la mayoría de
> módulos comerciales. Si no, agregar 4.7 kΩ a 3.3 V en SDA y SCL.

---

## 4. Conexión del módulo lector MicroSD (SPI)

| Pin módulo SD | Conectar a       |
|:-------------:|------------------|
| VCC           | +3.3V o +5V (según módulo) |
| GND           | GND              |
| SCK           | GP10 (SPI1 SCK)  |
| MOSI          | GP11 (SPI1 TX)   |
| MISO          | GP12 (SPI1 RX)   |
| CS            | GP13 (SPI1 CSn)  |

---

## 5. Etapa de potencia del ventilador (PWM + MOSFET)

```
              GP15 (PWM)                        +5V
                  |                              |
                  |                              |
                  +------/\/\/\------+         +-+-+
                       R = 220 Ω    |         |   |
                                    |    +----+ FAN
                                    |    |    |   |
                                    |    |    +-+-+
                                    |    |      |
                                  +-+-+  |      |
                              G |  ___ |  +----+----+
                                |  MOS |  |  Diodo  |
                              D |  FET |  | 1N4007  |
                                |      |  |   (k=+) |
                              S |______|  +----+----+
                                    |          |
                                    |          |
                  +------/\/\/\-----+          |
                       R = 10 kΩ   |          |
                                   |          |
                                  GND        GND
```

| Componente            | Valor / Referencia       | Conexión                       |
|-----------------------|--------------------------|--------------------------------|
| MOSFET (canal N)      | IRLZ44N (logic-level)    | G→GP15 vía 220Ω; D→ventilador (−); S→GND |
| Resistencia de gate   | 220 Ω, 1/4 W             | Entre GP15 y gate              |
| Resistencia pull-down | 10 kΩ, 1/4 W             | Entre gate y GND               |
| Diodo flyback         | 1N4007                   | Antiparalelo al ventilador (cátodo a +5V) |
| Ventilador DC         | 5 V, axial 40 mm         | (+) a +5V, (−) a drain del MOSFET |

---

## 6. Resumen de conexiones rápidas (Quick Reference)

| GPIO Pico W | Función      | Periférico         |
|:-----------:|--------------|---------------------|
| GP0         | UART0 TX     | DI del MAX485       |
| GP1         | UART0 RX     | RO del MAX485       |
| GP3         | GPIO out     | DE/RE_n del MAX485  |
| GP4         | I2C0 SDA     | SDA del OLED        |
| GP5         | I2C0 SCL     | SCL del OLED        |
| GP10        | SPI1 SCK     | SCK módulo SD       |
| GP11        | SPI1 MOSI    | MOSI módulo SD      |
| GP12        | SPI1 MISO    | MISO módulo SD      |
| GP13        | SPI1 CSn     | CS módulo SD        |
| GP15        | PWM 7B       | Gate del MOSFET IRLZ44N |
| ADC4        | Interno      | Sensor de temperatura RP2040 |

---

## 7. Alimentación

| Fuente    | Tensión | Uso                                          |
|-----------|---------|----------------------------------------------|
| USB Pico  | 5 V     | Entrada por USB (VBUS = pin 40)              |
| VBUS      | 5 V     | Salida 5V para MAX485, ventilador, módulo SD |
| 3V3 OUT   | 3.3 V   | Salida 3.3V para OLED                        |
| GND común | -       | Todos los GND deben estar unidos             |

---

## 8. Lista de comprobación antes de energizar

- [ ] Todos los GND están unidos al GND común.
- [ ] El MAX485 tiene VCC = 5 V (no 3.3 V).
- [ ] El OLED tiene VCC = 3.3 V.
- [ ] El pin GP15 tiene la resistencia pull-down de 10 kΩ a GND.
- [ ] El diodo 1N4007 está en antiparalelo al ventilador (cátodo al lado positivo).
- [ ] Las líneas A y B del RS-485 tienen terminación de 120 Ω y polarización.
- [ ] La microSD está formateada en FAT32.
- [ ] El SSID y la contraseña de Wi-Fi están actualizados en `config.h`.
- [ ] La IP del servidor backend está actualizada en `config.h`.
