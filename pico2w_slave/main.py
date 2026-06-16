"""
Esclavo Modbus RTU (MicroPython) para Raspberry Pi Pico 2 W
Lee un encoder en cuadratura del motor DC y expone RPM + conteo de
pulsos como Holding Registers, respondiendo al Pico W maestro por RS-485.

Conexiones en el Pico 2 W:
    MAX485 DI   -> GP0 (UART0 TX)
    MAX485 RO   -> GP1 (UART0 RX)
    MAX485 DE/RE-> GP3 (mismo pin, control de direccion)
    Encoder A   -> GP16
    Encoder B   -> GP17 (no usado para conteo, solo referencia)
    Encoder VCC -> 3V3 del Pico 2W (NO 5V)
    Encoder GND -> GND del Pico 2W

Sube este archivo como 'main.py' al Pico 2W para que arranque solo.
"""
import machine
import time

# --------------------------------------------------------------------------
#  Configuracion (debe coincidir con config.h del maestro)
# --------------------------------------------------------------------------
SLAVE_ADDR = 0x01
FC_READ_HOLDING = 0x03
NUM_REGS = 8

# AJUSTAR segun el datasheet real del motor/encoder. Si no se conoce,
# se reporta un valor proporcional (pulsos/seg) en lugar de RPM exacto.
PULSES_PER_REV = 360

UPDATE_PERIOD_MS = 500     # cada cuanto se recalcula RPM
REQUEST_LEN = 8            # el maestro siempre pide FC03 con trama de 8 bytes

# --------------------------------------------------------------------------
#  Hardware
# --------------------------------------------------------------------------
uart = machine.UART(0, baudrate=9600, tx=machine.Pin(0), rx=machine.Pin(1),
                     bits=8, parity=None, stop=1)

de_re = machine.Pin(3, machine.Pin.OUT)
de_re.value(0)  # arranca en modo recepcion

enc_a = machine.Pin(16, machine.Pin.IN, machine.Pin.PULL_UP)
enc_b = machine.Pin(17, machine.Pin.IN, machine.Pin.PULL_UP)

# --------------------------------------------------------------------------
#  Estado del encoder
# --------------------------------------------------------------------------
_pulse_count = 0


def _on_edge(pin):
    global _pulse_count
    _pulse_count += 1


enc_a.irq(trigger=machine.Pin.IRQ_RISING | machine.Pin.IRQ_FALLING,
          handler=_on_edge)

holding_regs = [0] * NUM_REGS
_last_snapshot = 0
_last_update_ms = time.ticks_ms()


def update_rpm():
    global _last_snapshot, _last_update_ms
    now = time.ticks_ms()
    dt_ms = time.ticks_diff(now, _last_update_ms)
    if dt_ms < UPDATE_PERIOD_MS:
        return
    delta = _pulse_count - _last_snapshot
    _last_snapshot = _pulse_count
    _last_update_ms = now

    revs = delta / PULSES_PER_REV
    minutes = (dt_ms / 1000.0) / 60.0
    rpm = int(revs / minutes) if minutes > 0 else 0

    holding_regs[0] = rpm & 0xFFFF
    holding_regs[1] = delta & 0xFFFF
    holding_regs[2] = _pulse_count & 0xFFFF  # contador acumulado (16 bits)


# --------------------------------------------------------------------------
#  Modbus RTU (CRC16, parseo de trama, respuesta FC03)
# --------------------------------------------------------------------------
def modbus_crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


def send_response(start_reg, qty):
    if qty > NUM_REGS:
        qty = NUM_REGS
    byte_count = qty * 2
    resp = bytearray()
    resp.append(SLAVE_ADDR)
    resp.append(FC_READ_HOLDING)
    resp.append(byte_count)
    for i in range(qty):
        idx = start_reg + i
        val = holding_regs[idx] if idx < NUM_REGS else 0
        resp.append((val >> 8) & 0xFF)
        resp.append(val & 0xFF)
    crc = modbus_crc16(resp)
    resp.append(crc & 0xFF)
    resp.append((crc >> 8) & 0xFF)

    de_re.value(1)          # habilita transmision en el MAX485
    time.sleep_us(50)
    uart.write(resp)
    time.sleep_us(2000)     # margen para vaciar la linea antes de soltarla
    de_re.value(0)          # vuelve a modo recepcion


def process_frame(frame):
    if len(frame) < 8:
        return
    crc_recv = frame[-2] | (frame[-1] << 8)
    crc_calc = modbus_crc16(frame[:-2])
    if crc_recv != crc_calc:
        return
    if frame[0] != SLAVE_ADDR or frame[1] != FC_READ_HOLDING:
        return
    start_reg = (frame[2] << 8) | frame[3]
    qty = (frame[4] << 8) | frame[5]
    send_response(start_reg, qty)


# --------------------------------------------------------------------------
#  Loop principal
# --------------------------------------------------------------------------
print("[esclavo] Listo. Direccion Modbus = 0x%02X" % SLAVE_ADDR)

rx_buf = bytearray()

while True:
    update_rpm()

    n = uart.any()
    if n:
        data = uart.read(n)
        if data:
            rx_buf.extend(data)

    if len(rx_buf) >= REQUEST_LEN:
        process_frame(rx_buf[:REQUEST_LEN])
        rx_buf = bytearray()

    time.sleep_ms(1)
