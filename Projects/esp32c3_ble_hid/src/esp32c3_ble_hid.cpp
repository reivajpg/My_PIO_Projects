#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BleGamepad.h>
#include "web_page.h"

// Credenciales WiFi
const char* ssid = "MyHome";
const char* password = "MyHome151_1";

BleGamepad bleGamepad("Mando Retro C3", "Fabricante DIY", 100);
WebServer server(80);

// Definición de pines
const int pinUp = 9;
const int pinDown = 1;
const int pinLeft = 2;
const int pinRight = 3;

// Estado de los botones web
bool webUp = false;
bool webDown = false;
bool webLeft = false;
bool webRight = false;

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleUpdate() {
  String btn = server.arg("btn");
  int val = server.arg("val").toInt();
  bool state = (val == 1);

  if (btn == "up") webUp = state;
  else if (btn == "down") webDown = state;
  else if (btn == "left") webLeft = state;
  else if (btn == "right") webRight = state;

  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(pinUp, INPUT_PULLUP);
  pinMode(pinDown, INPUT_PULLUP);
  pinMode(pinLeft, INPUT_PULLUP);
  pinMode(pinRight, INPUT_PULLUP);
  
  // Conectar WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  // Intentar conectar por 10 segundos, si no, continuar (para que funcione BLE sin WiFi si es necesario)
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println();
    Serial.print("Conectado. IP: ");
    Serial.println(WiFi.localIP());

    // Configurar Web Server
    server.on("/", handleRoot);
    server.on("/update", handleUpdate);
    server.begin();
    Serial.println("Servidor web iniciado");
  } else {
    Serial.println("\nNo se pudo conectar a WiFi. Continuando solo con BLE.");
  }

  bleGamepad.begin();
  Serial.println("BLE Gamepad iniciado");
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    server.handleClient();
  }
  //Serial.println(digitalRead());
  

  //Serial.println(bleGamepad.sendReport());

  if (bleGamepad.isConnected()) {
    bool up = !digitalRead(pinUp) || webUp;
    bool down = !digitalRead(pinDown) || webDown;
    bool left = !digitalRead(pinLeft) || webLeft;
    bool right = !digitalRead(pinRight) || webRight;

///////////////////////////////////////////////
/*
          uint8_t* buffer = bleGamepad.getOutputBuffer();
      Serial.print("Receive: ");
      
      for (int i = 0; i < 64; i++) 
      {
        Serial.printf("0x%X ",buffer[i]); // Print data from buffer
      }
      
      Serial.println("");
      delay(1000);
*///////////////////////////////////////////////

    // Lógica para el Hat Switch (Cruceta)
    if (up && right) bleGamepad.setHat1(DPAD_UP_RIGHT);
    else if (up && left) bleGamepad.setHat1(DPAD_UP_LEFT);
    else if (down && right) bleGamepad.setHat1(DPAD_DOWN_RIGHT);
    else if (down && left) bleGamepad.setHat1(DPAD_DOWN_LEFT);
    else if (up) bleGamepad.setHat1(DPAD_UP);
    else if (down) bleGamepad.setHat1(DPAD_DOWN);
    else if (left) bleGamepad.setHat1(DPAD_LEFT);
    else if (right) bleGamepad.setHat1(DPAD_RIGHT);
    else bleGamepad.setHat1(DPAD_CENTERED);

    bleGamepad.sendReport();
    delay(10); // Pequeña pausa para estabilidad
  }
}
