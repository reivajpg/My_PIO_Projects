AsyncWebServer server(80);

void handleFormPWM(AsyncWebServerRequest *request)
{
 String pwmValue = request->arg("pwmValue");
 
 Serial.print("PWM:\t");
 Serial.println(pwmValue);

 analogWrite(LED_BUILTIN, pwmValue.toInt());
 
 request->redirect("/");
}

void handleFormLed(AsyncWebServerRequest *request)
{
 String ledStatus = request->arg("status");
 
 Serial.print("Status:\t");
 Serial.println(ledStatus);

 digitalWrite(LED, ledStatus == "ON" ? LOW : HIGH);

 request->redirect("/");
}

void InitServer()
{
	server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
	//server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

	server.on("/LED_PWM", HTTP_POST, handleFormPWM);
	server.on("/LED", HTTP_POST, handleFormLed);

	server.onNotFound([](AsyncWebServerRequest *request) {
		request->send(400, "text/plain", "Not found");
	});

	server.begin();
    Serial.println("HTTP server started");
}