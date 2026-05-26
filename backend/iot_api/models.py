"""
Modelos de la app iot_api del proyecto IoT Edge Gateway
Autor: Brando Enrique Chavez Vergara
"""
from django.db import models


class TelemetryRecord(models.Model):
    """
    Cada registro POSTeado por el Pico W se persiste como una fila
    en esta tabla. Una entrada por segundo.
    """
    # Identificacion y tiempo
    received_at  = models.DateTimeField(auto_now_add=True, db_index=True)
    device_ts_ms = models.BigIntegerField(
        help_text="Tiempo en ms desde el boot del Pico W"
    )

    # Variables locales
    temp_internal = models.FloatField(
        help_text="Temperatura interna del RP2040 en grados Celsius"
    )
    pwm_duty = models.PositiveSmallIntegerField(
        help_text="Duty cycle PWM 0-255"
    )

    # Variables Modbus (8 holding registers)
    reg0 = models.IntegerField(default=0)
    reg1 = models.IntegerField(default=0)
    reg2 = models.IntegerField(default=0)
    reg3 = models.IntegerField(default=0)
    reg4 = models.IntegerField(default=0)
    reg5 = models.IntegerField(default=0)
    reg6 = models.IntegerField(default=0)
    reg7 = models.IntegerField(default=0)

    # Contadores
    modbus_errors = models.IntegerField(default=0)

    class Meta:
        ordering = ['-received_at']
        verbose_name = "Registro de telemetria"
        verbose_name_plural = "Registros de telemetria"

    def __str__(self):
        return (
            f"[{self.received_at:%Y-%m-%d %H:%M:%S}] "
            f"T={self.temp_internal:.1f}C  duty={self.pwm_duty}"
        )
