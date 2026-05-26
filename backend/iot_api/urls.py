"""
URL routing del app iot_api
"""
from django.urls import path
from . import views

app_name = 'iot_api'

urlpatterns = [
    path('telemetry/',        views.telemetry_ingest, name='telemetry_ingest'),
    path('telemetry/latest/', views.telemetry_latest, name='telemetry_latest'),
]
