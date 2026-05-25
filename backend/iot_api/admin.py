"""
Configuracion del admin para visualizar la telemetria recibida.
"""
from django.contrib import admin
from .models import TelemetryRecord


@admin.register(TelemetryRecord)
class TelemetryRecordAdmin(admin.ModelAdmin):
    list_display  = ('id', 'received_at', 'temp_internal', 'pwm_duty',
                     'modbus_errors', 'reg0', 'reg1')
    list_filter   = ('received_at',)
    search_fields = ('device_ts_ms',)
    date_hierarchy = 'received_at'
    readonly_fields = ('received_at',)
