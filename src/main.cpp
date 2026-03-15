#include <WiFi.h>
#include <WebServer.h>

// Configuración del Access Point
const char* ssid = "MedidorFrecuencia";  // Nombre de la red
const char* password = "12345678";        // Contraseña (mínimo 8 caracteres)

// O puedes crear una red abierta (sin contraseña)
// const char* ssid = "MedidorFrecuencia";
// const char* password = NULL;  // Red abierta

// Pines
const int INPUT_PIN = 4;  // Pin para medir frecuencia (GPIO4)

// Configuración de la cola circular
#define QUEUE_SIZE 50

// Variables para el timer y mediciones
volatile unsigned long lastTime = 0;
volatile float timeQueue[QUEUE_SIZE];
volatile int queueHead = 0;
volatile int queueTail = 0;
volatile int queueCount = 0;
volatile bool newDataAvailable = false;

// Variables para los cálculos de frecuencia
float currentFreq = 0;
float maxFreq = 0;
float minFreq = 0;
float avgFreq = 0;

// Servidor web
WebServer server(80);

// Mutex para proteger el acceso a la cola
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ISR: Se ejecuta en cada flanco de subida
void IRAM_ATTR onPulse() {
  unsigned long currentTime = micros();
  unsigned long timeDiff = currentTime - lastTime;
  lastTime = currentTime;
  
  if (timeDiff > 100) {
    portENTER_CRITICAL_ISR(&mux);
    
    timeQueue[queueHead] = timeDiff;
    queueHead = (queueHead + 1) % QUEUE_SIZE;
    
    if (queueCount < QUEUE_SIZE) {
      queueCount++;
    } else {
      queueTail = (queueTail + 1) % QUEUE_SIZE;
    }
    
    newDataAvailable = true;
    portEXIT_CRITICAL_ISR(&mux);
  }
}

// Función para calcular estadísticas de frecuencia
void calculateFreqStats() {
  portENTER_CRITICAL(&mux);
  
  if (queueCount > 0) {
    float sumFreq = 0;
    maxFreq = 0;
    minFreq = 0;
    
    int index = queueTail;
    for (int i = 0; i < queueCount; i++) {
      float period_us = timeQueue[index];
      float freq = 1000000.0 / period_us;
      
      sumFreq += freq;
      
      if (freq > maxFreq) maxFreq = freq;
      if (freq < minFreq) minFreq = freq;
      
      index = (index + 1) % QUEUE_SIZE;
    }
    
    avgFreq = sumFreq / queueCount;
    currentFreq = 1000000.0 / timeQueue[(queueHead - 1 + QUEUE_SIZE) % QUEUE_SIZE];
  }
  
  newDataAvailable = false;
  portEXIT_CRITICAL(&mux);
}

// Página web HTML (misma que antes)
String generateHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Medidor de Frecuencia ESP32</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        
        h1 {
            text-align: center;
            color: white;
            margin-bottom: 30px;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        .card {
            background: white;
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            transition: transform 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
        }
        
        .freq-display {
            text-align: center;
            font-size: 4em;
            font-weight: bold;
            color: #667eea;
            margin: 20px 0;
            font-family: monospace;
        }
        
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        
        .stat-item {
            text-align: center;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
        }
        
        .stat-label {
            font-size: 1em;
            color: #666;
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .stat-value {
            font-size: 2em;
            font-weight: bold;
            color: #333;
        }
        
        .stat-value.max { color: #28a745; }
        .stat-value.min { color: #dc3545; }
        .stat-value.avg { color: #007bff; }
        
        .queue-info {
            text-align: center;
            margin-top: 20px;
            padding: 10px;
            background: #e9ecef;
            border-radius: 5px;
            color: #495057;
        }
        
        .update-time {
            text-align: center;
            color: #6c757d;
            font-size: 0.9em;
            margin-top: 10px;
        }
        
        .status-led {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 5px;
        }
        
        .status-led.active {
            background: #28a745;
            animation: pulse 1s infinite;
        }
        
        .wifi-info {
            background: #e3f2fd;
            border-left: 4px solid #2196f3;
            padding: 10px;
            margin-bottom: 20px;
            border-radius: 5px;
            font-size: 0.9em;
        }
        
        @keyframes pulse {
            0% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.5; transform: scale(1.1); }
            100% { opacity: 1; transform: scale(1); }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔍 Medidor de Frecuencia ESP32</h1>
        
        <div class="wifi-info" id="wifiInfo">
            Conectado a la red del ESP32
        </div>
        
        <div class="card">
            <div style="text-align: center; margin-bottom: 20px;">
                <span class="status-led active"></span>
                <span style="color: #666;">Midiendo en tiempo real</span>
            </div>
            
            <div class="freq-display" id="currentFreq">0.00</div>
            <div style="text-align: center; color: #666; margin-bottom: 30px;">
                Frecuencia Actual (Hz)
            </div>
            
            <div class="stats-grid">
                <div class="stat-item">
                    <div class="stat-label">Máxima</div>
                    <div class="stat-value max" id="maxFreq">0.00</div>
                    <div>Hz</div>
                </div>
                
                <div class="stat-item">
                    <div class="stat-label">Mínima</div>
                    <div class="stat-value min" id="minFreq">0.00</div>
                    <div>Hz</div>
                </div>
                
                <div class="stat-item">
                    <div class="stat-label">Promedio</div>
                    <div class="stat-value avg" id="avgFreq">0.00</div>
                    <div>Hz</div>
                </div>
            </div>
            
            <div class="queue-info" id="queueInfo">
                Muestras en cola: 0 / )rawliteral" + String(QUEUE_SIZE) + R"rawliteral(
            </div>
            
            <div class="update-time" id="updateTime">
                Última actualización: --:--:--
            </div>
        </div>
        
        <div class="card" style="text-align: center;">
            <h3 style="color: #333; margin-bottom: 15px;">📊 Información del Sistema</h3>
            <p style="color: #666; margin-bottom: 10px;">
                Tamaño de cola circular: )rawliteral" + String(QUEUE_SIZE) + R"rawliteral( muestras
            </p>
            <p style="color: #666;">
                Pin de entrada: GPIO )rawliteral" + String(INPUT_PIN) + R"rawliteral(
            </p>
            <p style="color: #666;">
                Red WiFi: )rawliteral" + String(ssid) + R"rawliteral(
            </p>
        </div>
    </div>
    
    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('currentFreq').textContent = data.current.toFixed(2);
                    document.getElementById('maxFreq').textContent = data.max.toFixed(2);
                    document.getElementById('minFreq').textContent = data.min.toFixed(2);
                    document.getElementById('avgFreq').textContent = data.avg.toFixed(2);
                    document.getElementById('queueInfo').innerHTML = 
                        `Muestras en cola: ${data.queueCount} / ${data.queueSize}`;
                    
                    const now = new Date();
                    const timeStr = now.toLocaleTimeString();
                    document.getElementById('updateTime').textContent = 
                        `Última actualización: ${timeStr}`;
                    
                    document.getElementById('wifiInfo').innerHTML = 
                        `Red: ${data.wifiSSID} | IP: ${data.wifiIP}`;
                })
                .catch(error => console.error('Error:', error));
        }
        
        setInterval(updateData, 100);
        updateData();
    </script>
</body>
</html>
)rawliteral";
  return html;
}

// Handler para la página principal
void handleRoot() {
  server.send(200, "text/html", generateHTML());
}

// Handler para obtener datos en JSON
void handleData() {
  calculateFreqStats();
  
  String json = "{";
  json += "\"current\":" + String(currentFreq, 2) + ",";
  json += "\"max\":" + String(maxFreq, 2) + ",";
  json += "\"min\":" + String(minFreq, 2) + ",";
  json += "\"avg\":" + String(avgFreq, 2) + ",";
  json += "\"queueCount\":" + String(queueCount) + ",";
  json += "\"queueSize\":" + String(QUEUE_SIZE) + ",";
  json += "\"wifiSSID\":\"" + String(WiFi.softAPSSID()) + "\",";  // Nombre de la red
  json += "\"wifiIP\":\"" + WiFi.softAPIP().toString() + "\"";     // IP del AP
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handler para 404
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Iniciando Medidor de Frecuencia en modo Access Point...");
  
  // Configurar pin de entrada
  pinMode(INPUT_PIN, INPUT_PULLUP);
  
  // Inicializar cola circular
  for (int i = 0; i < QUEUE_SIZE; i++) {
    timeQueue[i] = 0;
  }
  
  // Configurar interrupción
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN), onPulse, RISING);
  
  // Configurar modo Access Point
  Serial.println("Configurando Access Point...");
  
  // Opción 1: Con contraseña
  WiFi.softAP(ssid, password);
  
  // Opción 2: Red abierta (comenta la de arriba y descomenta esta)
  // WiFi.softAP(ssid);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("Access Point iniciado!");
  Serial.print("Nombre de red (SSID): ");
  Serial.println(ssid);
  if (password != NULL) {
    Serial.print("Contraseña: ");
    Serial.println(password);
  } else {
    Serial.println("Red abierta (sin contraseña)");
  }
  Serial.print("Dirección IP del ESP32: ");
  Serial.println(IP);
  Serial.println("Conectate a la red WiFi y abre http://" + IP.toString() + " en tu navegador");
  
  // Configurar rutas del servidor
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);
  
  // Iniciar servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado");
  
  // Mostrar información del pin
  Serial.println("Pin de entrada: GPIO" + String(INPUT_PIN));
  Serial.println("Esperando señal...");
}

void loop() {
  server.handleClient();
  
  // Mostrar información en Serial (opcional)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    calculateFreqStats();
    Serial.printf("Frec: %.2f Hz | Max: %.2f | Min: %.2f | Prom: %.2f | Muestras: %d\n", 
                  currentFreq, maxFreq, minFreq, avgFreq, queueCount);
    lastPrint = millis();
  }
}