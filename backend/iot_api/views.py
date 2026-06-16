"""
Vistas de la API REST del proyecto IoT Edge Gateway
Autor: Brando Enrique Chavez Vergara
"""
import json
import logging
from django.http import JsonResponse, HttpResponseBadRequest
from django.shortcuts import render
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_http_methods

from .models import TelemetryRecord

logger = logging.getLogger(__name__)


def dashboard(request):
    """Vista GET / — dashboard en vivo con fondo azul oscuro."""
    return render(request, 'iot_api/dashboard.html')


@csrf_exempt
@require_http_methods(["POST"])
def telemetry_ingest(request):
    """
    Endpoint POST /api/telemetry/
    Recibe el JSON enviado por el Pico W y persiste el registro en la BD.

    Formato esperado del JSON:
    {
      "ts": 12345678,
      "temp": 32.5,
      "duty": 64,
      "regs": [r0, r1, r2, r3, r4, r5, r6, r7],
      "errors": 0
    }
    """
    try:
        payload = json.loads(request.body)
    except json.JSONDecodeError as e:
        logger.warning(f"JSON invalido: {e}")
        return HttpResponseBadRequest(f"JSON invalido: {e}")

    try:
        regs = payload.get("regs", [0] * 8)
        regs = (regs + [0] * 8)[:8]   # garantiza 8 elementos

        record = TelemetryRecord.objects.create(
            device_ts_ms  = int(payload.get("ts", 0)),
            temp_internal = float(payload.get("temp", 0.0)),
            pwm_duty      = int(payload.get("duty", 0)),
            reg0=regs[0], reg1=regs[1], reg2=regs[2], reg3=regs[3],
            reg4=regs[4], reg5=regs[5], reg6=regs[6], reg7=regs[7],
            modbus_errors = int(payload.get("errors", 0)),
        )
        logger.info(f"Telemetria recibida: id={record.id} T={record.temp_internal}")
        return JsonResponse({"status": "ok", "id": record.id})

    except (TypeError, ValueError, KeyError) as e:
        logger.error(f"Payload mal formado: {e}")
        return HttpResponseBadRequest(f"Payload mal formado: {e}")


@require_http_methods(["GET"])
def telemetry_latest(request):
    """
    Endpoint GET /api/telemetry/latest/
    Devuelve las ultimas 50 muestras (orden descendente).
    """
    qs = TelemetryRecord.objects.all()[:50]
    data = [{
        "id":            r.id,
        "received_at":   r.received_at.isoformat(),
        "device_ts_ms":  r.device_ts_ms,
        "temp_internal": r.temp_internal,
        "pwm_duty":      r.pwm_duty,
        "regs":          [r.reg0, r.reg1, r.reg2, r.reg3,
                          r.reg4, r.reg5, r.reg6, r.reg7],
        "modbus_errors": r.modbus_errors,
    } for r in qs]
    return JsonResponse({"count": len(data), "results": data})
