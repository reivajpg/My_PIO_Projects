#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.

unsigned int modo=0;

void wifi_setup( unsigned int modo) {
  switch(modo){
    case 0:{
      //Serial.println("");
      WiFi.mode(WIFI_STA);
      WiFi.begin(sta_ssid, sta_password);
      #ifdef STATIC_IP
        WiFi.config(ip, gateway, subnet);
      #endif
      unsigned long connect_start = millis();
      while (WiFi.status() != WL_CONNECTED) //If no connection is made within WIFI_TIMEOUT, ESP gets resettet.
        { 
          delay(500); Serial.print('.');
          if(millis() - connect_start > WIFI_TIMEOUT) {
            Serial.println();
            Serial.print("Tried ");
            Serial.print(WIFI_TIMEOUT);
            Serial.print("ms. Cambiando a modo AP.");
            //ESP_RESET;
            wifi_setup(1);
            break;
          }
        }

      //Serial.println("");
      Serial.print("Iniciado modo STA:\t");
      Serial.println(sta_ssid);
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());
      }
      break;
    case 1:{
      //Serial.println("");
      WiFi.mode(WIFI_AP);
      while(!WiFi.softAP(ap_ssid, ap_password))
        {
          Serial.println("."); delay(100);
        }
      #ifdef STATIC_IP  
        WiFi.config(ip, gateway, subnet);
      #endif
      //Serial.println("");
      Serial.print("Iniciado modo AP:\t");
      Serial.println(ap_ssid);
      Serial.print("IP address:\t");
      Serial.println(WiFi.softAPIP());
      }
      break;
  }
}

/*
void ConnectWiFi_STA()
{
   Serial.println("");
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   #ifdef STATIC_IP  
    WiFi.config(ip, gateway, subnet);
   #endif
   while (WiFi.status() != WL_CONNECTED) 
   { 
     delay(100);  
     Serial.print('.'); 
   }
 
   Serial.println("");
   Serial.print("Iniciado modo STA:\t");
   Serial.println(ssid);
   Serial.print("IP address:\t");
   Serial.println(WiFi.localIP());
}

void ConnectWiFi_AP()
{ 
   Serial.println("");
   WiFi.mode(WIFI_AP);
   while(!WiFi.softAP(ssid, password))
   {
     Serial.println(".");
     delay(100);
   }
   #ifdef STATIC_IP  
    WiFi.config(ip, gateway, subnet);
   #endif
   Serial.println("");
   Serial.print("Iniciado modo AP:\t");
   Serial.println(ssid);
   Serial.print("IP address:\t");
   Serial.println(WiFi.softAPIP());
}
*/



/*
 * Init WiFi. If no connection is made within WIFI_TIMEOUT, ESP gets resettet.
 */
#ifdef ARDUINO_ARCH_ESP32
  #include <WiFi.h>
  #include <WebServer.h>
  #define WEB_SERVER WebServer
  #define ESP_RESET ESP.restart()
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #define WEB_SERVER ESP8266WebServer
  #define ESP_RESET ESP.reset()
#endif


/**
 * 
#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.

  if(now - last_wifi_check_time > WIFI_TIMEOUT) {
    Serial.print("Checking WiFi... ");
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Reconnecting...");
      wifi_setup();
    } else {
      Serial.println("OK");
    }
    last_wifi_check_time = now;
  }
**/