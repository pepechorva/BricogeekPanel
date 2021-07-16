#if defined(ESP8266)
#include <ESPAsyncWiFiManager.h> 
#include <FS.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#else
#include <WiFiManager.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#endif


#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>




/*
 * @brief Web de control de 2 tiras led para iluminar un doble panel de metacrilato
 * 
 * Por defecto, se levanta un portal cautivo con SSID "ESP32" y pass "12345678"
 * Además, los leds están apagados y el brillo a 0
 * Con el portal cautivo, los datos de ls wifi quedan guardados en la flash del ESP
 * Se puede forzar a que los olvide siempre poniendo CLEARCREDENTIALS a 1
*/


#if defined(ESP8266)
#define LED_PIN_FRONT D2
#define LED_PIN_BACK D3
#else
#define LED_PIN_FRONT 13
#define LED_PIN_BACK 12
#endif

#define LED_COUNT  20
#define BRIGHTNESS 0 

#define WEBSERVER 80
#define WEBSOCKET 81
#define WEBNOTFOUND 400
#define WEBOK 200

#define JSONSIZE 200

#define DEFAULTSSID "ESP32"
#define DEFAULTPASSW "12345678"

#define CLEARCREDENTIALS 0 //0 for persistance


#if defined(ESP8266)
AsyncWebServer server(WEBSERVER);
DNSServer dns;
AsyncWiFiManager wm(&server,&dns);
#else
WiFiManager wm;
AsyncWebServer server(WEBSERVER);
WebSocketsServer webSocket(WEBSOCKET);
#endif


Adafruit_NeoPixel frontStrip(LED_COUNT, LED_PIN_FRONT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel backStrip(LED_COUNT, LED_PIN_BACK, NEO_GRB + NEO_KHZ800);

/*
int GetIdFromURL(AsyncWebServerRequest *request, String root)
{
  String string_id = request->url();
  string_id.replace(root, "");
  int id = string_id.toInt();
  return id;
}*/

String GetBodyContent(uint8_t *data, size_t len)
{
  String content = "";
  for (size_t i = 0; i < len; i++) {
    content .concat((char)data[i]);
  }
  return content;
}

/*
void getData(AsyncWebServerRequest *request) //TODO
{
  //TO-DO: está tal como está en el ejemplo de la web
  AsyncResponseStream *response = request->beginResponseStream("application/json");
   // obtendríamos datos de GPIO, estado...
   StaticJsonDocument<300> jsonDoc;
   jsonDoc["id"] = random(0,10);
   jsonDoc["status"] = random(0,2);
   serializeJson(jsonDoc, *response);
   request->send(response);
}*/

 void setData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String bodyContent = GetBodyContent(data, len);
  StaticJsonDocument<JSONSIZE> doc;
  DeserializationError error = deserializeJson(doc, bodyContent);
  if (error) 
  { 
    request->send(WEBNOTFOUND); return;
  }
 
  int rf = doc["rf"];
  int gf = doc["gf"];
  int bf = doc["bf"];
  int brightf = doc["brightf"];
  int rb = doc["rb"];
  int gb = doc["gb"];
  int bb = doc["bb"];
  int brightb = doc["brightb"];

  ledStripFront(rf, gf, bf, brightf);
  Serial.printf("Front RGB  R = %d, G = %d, B = %d, Brillo = %d\n", rf, gf, bf, brightf);
  Serial.printf("Back  RGB  R = %d, G = %d, B = %d, Brillo = %d\n", rb, gb, bb, brightb);

  ledStripBack(rb, gb, bb, brightb);
  request->send(WEBOK);
}

void ledStripFront(byte R, byte G, byte B, byte brightness)
{
  frontStrip.setBrightness(brightness);
  for ( int i = 0; i < LED_COUNT; i++)
  {
    frontStrip.setPixelColor(i, frontStrip.Color(R, G, B));
    delay(10);
  }
  frontStrip.show();
}

void ledStripBack(byte R, byte G, byte B, byte brightness)
{
  backStrip.setBrightness(brightness);
  for ( int i = 0; i < LED_COUNT; i++)
  {
    backStrip.setPixelColor(i, backStrip.Color(R, G, B));
    delay(10);
  }
  backStrip.show();
}

void InitWifi()
{
  if(CLEARCREDENTIALS)
  {
  //reset saved settings *ONLY FOR DEBUGGING*
    wm.resetSettings();
  }
  //Try to connect WiFi, then create AP
  wm.autoConnect(DEFAULTSSID, DEFAULTPASSW);

  //the library is blocking. Once connected, the program continues
  Serial.println("ESP32 is connected to Wi-Fi network");
}

void InitServer()
{
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  //server.on("/values", HTTP_GET, getData);
  server.on("/values", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setData);
  
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(WEBNOTFOUND, "text/plain", "Not found");
  });
  server.begin();
  Serial.println("HTTP server started");
}


void InitLEDs()
{
  frontStrip.begin();
  backStrip.begin();
  frontStrip.clear();
  backStrip.clear();
  frontStrip.show();
  backStrip.show();
  frontStrip.setBrightness(BRIGHTNESS);
  backStrip.setBrightness(BRIGHTNESS);
  for ( int i = 0; i < LED_COUNT; i++)
  {
    frontStrip.setPixelColor(i, frontStrip.Color(0, 0, 0));
    backStrip.setPixelColor(i, backStrip.Color(0, 0, 0));
    delay(10);
  }
  frontStrip.show();
  backStrip.show();
}

void setup(void)
{
  Serial.begin(115200);
  SPIFFS.begin();
  InitLEDs();
  InitWifi();
  InitServer();
}

void loop(void)
{
}
