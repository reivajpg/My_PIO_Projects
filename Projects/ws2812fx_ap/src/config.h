const char* hostname = "NODEMCU_V3";

const char* ssid     = "IxDM";
const char* password = "qwertyui";
/////////////////////////////////////////
//const char* ssid = "Mi_AP";
//const char* password = "qwertyui";


////////////////////////////////////////////////////////////////////////

#define STATIC_IP                       // uncomment for static IP, set IP below
#ifdef STATIC_IP
  IPAddress ip(192,168,0,1);
  IPAddress gateway(192,168,0,1);
  IPAddress subnet(255,255,255,0);
#endif

// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

//////////////////////////////////////////////////
//IPAddress ip(192, 168, 1, 108);
//IPAddress gateway(192, 168, 1, 1);
//IPAddress subnet(255, 255, 255, 0);
