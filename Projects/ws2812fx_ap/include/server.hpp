void InitServer()
{
  Serial.println("HTTP server setup");
  server.on("/", srv_handle_index_html);
  server.on("/main.js", srv_handle_main_js);
  server.on("/modes", srv_handle_modes);
  server.on("/set", srv_handle_set);
  server.onNotFound(srv_handle_not_found);
  server.begin();
  Serial.println("HTTP server started.");

  Serial.println("ready!");
}

/*
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
*/

/*
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
*/