#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h> //<FS.h>
#include <ArduinoOTA.h>

#define LED 16

#include "config.h"  // Sustituir con datos de vuestra red
#include "server.hpp"
#include "ESP8266_Utils.hpp"
#include "ota.hpp"

void setup(void)
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);

	Serial.begin(115200);
	LittleFS.begin(); //SPIFFS.begin();
	
	Serial.println("Wifi setup");
  	wifi_setup(0); // 0=STA 1=AP

	InitServer();
}

void loop(void)
{
}
