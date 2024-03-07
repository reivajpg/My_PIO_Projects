const char* hostname = "NODEMCU_V3";

//////////STA_Mode//////////
const char* sta_ssid     = "IxDM";
const char* sta_password = "qwertyui";

//////////AP_Mode//////////
const char* ap_ssid = "Mi_AP";
const char* ap_password = "qwertyui";


////////////////////////////////////////////////////////////////////////

//#define STATIC_IP                       // uncomment for static IP, set IP below
#ifdef STATIC_IP
  IPAddress ip(192,168,0,1);
  IPAddress gateway(192,168,0,1);
  IPAddress subnet(255,255,255,0);
#endif

// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
//#define min(a,b) ((a)<(b)?(a):(b))
//#define max(a,b) ((a)>(b)?(a):(b))

//////////////////////////////////////////////////
//IPAddress ip(192, 168, 1, 108);
//IPAddress gateway(192, 168, 1, 1);
//IPAddress subnet(255, 255, 255, 0);
