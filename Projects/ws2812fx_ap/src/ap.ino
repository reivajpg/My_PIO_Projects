#include <ESP8266WiFi.h>
#include <WS2812FX.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

extern const char index_html[];

#define LED_PIN   2 //D1 // digital pin used to drive the LED strip (esp8266)
#define LED_COUNT 3 //30 // number of LEDs on the strip


WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800); //NEO_GRBW for RGBW leds can be changed in to GRB
ESP8266WebServer server(80);
const char *ssid = "Ligth_AP"; // AP ssid name
const char *password = "qwertyui"; //the password should be 8 char or more

void setup() {
  Serial.begin(115200);
  Serial.println("\r\n");

  // init WiFi
 
WiFi.softAP(ssid, password); 

   WiFi.mode(WIFI_AP);
  Serial.print("\nPoint your browser to ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart([]() {
    Serial.println("OTA start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA end");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  /* init web server */
  // return the index.html file
  server.on("/", [](){
    server.send_P(200, "text/html", index_html);
  });

  // send the segment info in JSON format
  server.on("/getsegments", [](){
    DynamicJsonBuffer jsonBuffer(1000);
    JsonObject& root = jsonBuffer.createObject();
    root["pin"] = ws2812fx.getPin();
    root["numPixels"] = ws2812fx.numPixels();
    root["numSegments"] = ws2812fx.getNumSegments();
    JsonArray& jsonSegments = root.createNestedArray("segments");

    WS2812FX::segment* segments = ws2812fx.getSegments();
    for(int i=0; i<ws2812fx.getNumSegments(); i++) {
      WS2812FX::segment seg = segments[i];
      JsonObject& jsonSegment = jsonBuffer.createObject();
      jsonSegment["start"] = seg.start;
      jsonSegment["stop"] = seg.stop;
      jsonSegment["mode"] = seg.mode;
      jsonSegment["speed"] = seg.speed;
      jsonSegment["reverse"] = seg.options & REVERSE ? true: false;
      JsonArray& jsonColors = jsonSegment.createNestedArray("colors");
      jsonColors.add(seg.colors[0]); // the web interface expects three color values
      jsonColors.add(seg.colors[1]);
      jsonColors.add(seg.colors[2]);
      jsonSegments.add(jsonSegment);
    }
    // root.printTo(Serial);

    int bufferSize = root.measureLength() + 1;
    char *json = (char*)malloc(sizeof(char) * (bufferSize));
    root.printTo(json, sizeof(char) * bufferSize);
    server.send(200, "application/json", json);
    free(json);
  });

  // receive the segment info in JSON format and setup the WS2812 strip
  server.on("/setsegments", HTTP_POST, [](){
    String data = server.arg("plain");
    data = data.substring(1, data.length() - 1); // remove the surrounding quotes
    data.replace("\\\"", "\""); // replace all the backslash quotes with just quotes
//Serial.println(data);
    DynamicJsonBuffer jsonBuffer(1000);
    JsonObject& root = jsonBuffer.parseObject(data);
    if (root.success()) {
      ws2812fx.stop();
      ws2812fx.setLength(root["numPixels"]);
      ws2812fx.stop(); // reset strip again in case length was increased
      ws2812fx.setNumSegments(1); // reset number of segments
      JsonArray& segments = root["segments"];
      for (int i = 0; i< segments.size(); i++){
        JsonObject& seg = segments[i];
        JsonArray& colors = seg["colors"];
        // the web interface sends three color values
        uint32_t _colors[NUM_COLORS] = {colors[0], colors[1], colors[2]};
        bool reverse = seg["reverse"];
        ws2812fx.setSegment(i, seg["start"], seg["stop"], seg["mode"], _colors, seg["speed"], reverse ? REVERSE : NO_OPTIONS);
      }
      ws2812fx.start();
    }
    server.send(200, "text/plain", "OK");
  });

  // send the WS2812 mode info
  server.on("/getmodes", [](){
    DynamicJsonBuffer jsonBuffer(1000);
    JsonArray& root = jsonBuffer.createArray();
    for(uint8_t i=0; i < ws2812fx.getModeCount(); i++) {
      root.add(ws2812fx.getModeName(i));
    }

    int bufferSize = root.measureLength() + 1;
    char *json = (char*)malloc(sizeof(char) * (bufferSize));
    root.printTo(json, sizeof(char) * bufferSize);
    server.send(200, "application/json", json);
    free(json);
  });

  server.onNotFound([](){
    server.send(404, "text/plain", "Page not found");
  });

  // start the web server
  server.begin();

  // init LED strip with some default segments ive seted to static white at the start for my audi S6C5 interior lights
  ws2812fx.init();
  ws2812fx.setBrightness(255); // set to full brightness
  // parameters:  index, start,        stop,         mode,                              colors, speed, reverse
  ws2812fx.setSegment(0,     0, LED_COUNT-1, FX_MODE_STATIC, (const uint32_t[]) {0xffffff, 0xffffff, 0xffffff},  1000, false);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
  server.handleClient();
  ArduinoOTA.handle();
}

