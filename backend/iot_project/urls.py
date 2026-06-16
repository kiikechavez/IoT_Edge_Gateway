"""
URL routing raiz del proyecto IoT Edge Gateway
"""
from django.contrib import admin
from django.urls import path, include
from iot_api import views as iot_views

urlpatterns = [
    path('',      iot_views.dashboard,    name='dashboard'),
    path('api/',  include('iot_api.urls')),
    path('admin/', admin.site.urls),
]
