#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h> //<FS.h>

#define LED 16

#include "config.h"  // Sustituir con datos de vuestra red
#include "Server.hpp"
#include "ESP8266_Utils.hpp"

void setup(void)
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);

	Serial.begin(115200);
	LittleFS.begin(); //SPIFFS.begin();
	
	ConnectWiFi_AP();

	InitServer();
}

void loop(void)
{
}
