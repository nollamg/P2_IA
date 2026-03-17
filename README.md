# Medidor de Frecuencia WiFi con ESP32

![ESP32](https://img.shields.io/badge/ESP32-Frequency_Meter-blue)
![WiFi AP](https://img.shields.io/badge/WiFi-Access_Point-orange)
![Interrupt](https://img.shields.io/badge/Interrupt-Driven-green)

Medidor de frecuencia de alta precisión basado en ESP32 con interfaz web WiFi en tiempo real. Ideal para medir señales digitales hasta varios kHz.

## 📋 Descripción

Este proyecto convierte tu ESP32 en un medidor de frecuencia profesional con las siguientes características:

- **Medición en tiempo real** mediante interrupciones por hardware
- **Cola circular** de 50 muestras para estadísticas precisas
- **Servidor web integrado** con interfaz moderna y responsiva
- **Modo Access Point** (no necesita router externo)
- **Actualización cada 100ms** vía AJAX/JSON

## 🚀 Características Principales

| Característica | Descripción |
|----------------|-------------|
| **Rango medición** | Desde <1 Hz hasta ~100 kHz |
| **Precisión** | Alta (basada en micros()) |
| **Interfaz** | Web responsive con diseño moderno |
| **Red** | Access Point propio (SSID: "MedidorFrecuencia") |
| **Actualización** | Tiempo real cada 100ms |
| **Estadísticas** | Actual, Máxima, Mínima, Promedio |

## 🔧 Requisitos

### Hardware
- ESP32 (cualquier modelo)
- Fuente de señal (generador de funciones, sensor, etc.)
- Cables para conectar señal al GPIO4
- Cable USB para programación

### Software
- PlatformIO o Arduino IDE
- Navegador web (Chrome, Firefox, Edge, etc.)
- Dispositivo con WiFi (teléfono, tablet, PC)

## 📌 Conexiones

| Señal | Pin ESP32 |
|-------|-----------|
| Entrada de frecuencia | GPIO4 |
| GND (común) | GND |

*La señal debe ser digital (0-3.3V) y conectarse al GPIO4 con referencia a GND*

## 💻 Instalación Rápida

```bash
git clone https://github.com/tu-usuario/esp32-frequency-meter.git
cd esp32-frequency-meter
# Abrir en PlatformIO o Arduino IDE
# Subir a ESP32
Credenciales WiFi (modificables en código)
cpp
const char* ssid = "MedidorFrecuencia";  // Cambia el nombre
const char* password = "12345678";        // Cambia la contraseña
// Para red abierta: const char* password = NULL;
📊 Uso
1. Iniciar el ESP32
text
Monitor serial (115200 baudios):
Iniciando Medidor de Frecuencia en modo Access Point...
Access Point iniciado!
Nombre de red: MedidorFrecuencia
Contraseña: 12345678
IP del ESP32: 192.168.4.1
2. Conectarse al WiFi
Busca la red MedidorFrecuencia en tu dispositivo

Conecta con contraseña 12345678

3. Acceder a la interfaz web
Abre el navegador

Ve a http://192.168.4.1

¡Listo! Verás el medidor funcionando

🖥️ Interfaz Web
La interfaz muestra en tiempo real:

https://via.placeholder.com/800x400?text=Interfaz+Medidor+Frecuencia

Frecuencia Actual (display grande)

Frecuencia Máxima (verde)

Frecuencia Mínima (rojo)

Frecuencia Promedio (azul)

Estado de la cola circular

Información del sistema

📝 Ejemplo de Salida
Monitor Serial
text
Frec: 1000.25 Hz | Max: 1001.30 | Min: 999.80 | Prom: 1000.15 | Muestras: 50
Frec: 1000.50 Hz | Max: 1001.30 | Min: 999.80 | Prom: 1000.18 | Muestras: 50
Frec: 1000.10 Hz | Max: 1001.30 | Min: 999.80 | Prom: 1000.16 | Muestras: 50
