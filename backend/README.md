# Backend Django — IoT Edge Gateway

API REST que recibe los POSTs de telemetría desde la Raspberry Pi Pico W
y los persiste en una base de datos relacional (SQL Server o SQLite para
pruebas).

---

## Requisitos

- Python 3.10+
- Django 4.2+
- (Opcional) Microsoft SQL Server + `mssql-django` para producción
- SQLite para desarrollo (incluido en Python)

---

## Instalación rápida (desarrollo con SQLite)

```bash
cd backend/
python -m venv venv
source venv/bin/activate           # Linux/Mac
# venv\Scripts\activate            # Windows

pip install django==4.2.* djangorestframework

python manage.py makemigrations iot_api
python manage.py migrate
python manage.py createsuperuser
python manage.py runserver 0.0.0.0:8000
```

Accede al admin en `http://<ip-del-servidor>:8000/admin/`.

---

## Producción con SQL Server

```bash
pip install mssql-django pyodbc
```

En `settings.py`:

```python
DATABASES = {
    'default': {
        'ENGINE':   'mssql',
        'NAME':     'iot_edge_db',
        'USER':     'sa',
        'PASSWORD': 'TuPasswordFuerte!',
        'HOST':     'localhost',
        'PORT':     '1433',
        'OPTIONS': {
            'driver': 'ODBC Driver 17 for SQL Server',
        },
    }
}
```

---

## Endpoints

| Método | URL                          | Descripción                          |
|--------|------------------------------|--------------------------------------|
| POST   | `/api/telemetry/`            | Recibe el JSON del Pico W            |
| GET    | `/api/telemetry/latest/`     | Devuelve las últimas 50 muestras     |
| GET    | `/admin/`                    | Interfaz administrativa de Django    |

---

## Ejemplo de payload

```json
{
  "ts": 12345678,
  "temp": 32.5,
  "duty": 64,
  "regs": [100, 200, 0, 0, 0, 0, 0, 0],
  "errors": 0
}
```

## Verificación con curl

```bash
curl -X POST http://localhost:8000/api/telemetry/ \
     -H "Content-Type: application/json" \
     -d '{"ts":1,"temp":25.0,"duty":0,"regs":[0,0,0,0,0,0,0,0],"errors":0}'
```
