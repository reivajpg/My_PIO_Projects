#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include <SPI.h>

// Credenciales Wi-Fi (Cámbialas por las tuyas)
const char* ssid = "MyHome";
const char* password = "MyHome151_1";

// Configuración del servidor NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600; //7200;  // Ajuste de zona horaria (Ejemplo: -3 horas * 3600 = -10800 para Argentina/Chile/Uruguay)
const int   daylightOffset_sec = 0;  // Horario de verano (3600 si aplica)

WebServer server(80);

// Página Web Embebida (HTML/CSS/JS)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Reloj NTP ESP32-C3</title>
  <style>
    body {
      background-color: #1a1a1a;
      color: #e0e0e0;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      margin: 0;
    }
    .container {
      background: #2d2d2d;
      padding: 40px;
      border-radius: 15px;
      box-shadow: 0 10px 25px rgba(0,0,0,0.5);
      text-align: center;
    }
    h1 { margin: 0 0 20px 0; font-size: 1.2rem; text-transform: uppercase; letter-spacing: 2px; color: #888; }
    #clock {
      font-size: 4rem;
      font-weight: 700;
      color: #00d2ff;
      text-shadow: 0 0 20px rgba(0, 210, 255, 0.4);
      font-variant-numeric: tabular-nums;
    }
    #date {
      margin-top: 15px;
      font-size: 1.2rem;
      color: #aaa;
    }
    .status {
      margin-top: 30px;
      font-size: 0.8rem;
      color: #555;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Reloj ESP32-C3</h1>
    <div id="clock">--:--:--</div>
    <div id="date">Sincronizando...</div>
    <div class="status">Sincronizado vía NTP</div>
  </div>

  <script>
    function updateClock() {
      fetch('/time_json')
        .then(response => response.json())
        .then(data => {
          document.getElementById('clock').innerHTML = data.time;
          document.getElementById('date').innerHTML = data.date;
        })
        .catch(err => console.error('Error fetching time:', err));
    }

    // Actualizar cada segundo
    setInterval(updateClock, 1000);
    // Primera carga inmediata
    updateClock();
  </script>
</body>
</html>
)rawliteral";

// Función para obtener la hora local formateada
String getLocalTimeStr() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Error NTP";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

// Función para obtener la fecha local formateada
String getLocalDateStr() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "--/--/--";
  }
  char dateStringBuff[50];
  strftime(dateStringBuff, sizeof(dateStringBuff), "%d/%m/%Y", &timeinfo);
  return String(dateStringBuff);
}

// Handler: Página Principal
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// Handler: API JSON de Tiempo
void handleTimeJson() {
  String timeStr = getLocalTimeStr();
  String dateStr = getLocalDateStr();

  // Crear JSON manual simple
  String json = "{";
  json += "\"time\": \"" + timeStr + "\",";
  json += "\"date\": \"" + dateStr + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  // Conectar a Wi-Fi
  Serial.printf("Conectando a %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONECTADO");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Iniciar y configurar NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Configurar Rutas del Servidor Web
  server.on("/", handleRoot);
  server.on("/time_json", handleTimeJson);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
  delay(2); // Pequeña pausa para estabilidad
}