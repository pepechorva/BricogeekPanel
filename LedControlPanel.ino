#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Adafruit_NeoPixel.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
//#include "Server.hpp"
//#include "Leds.hpp"
//#include "webSocketsled.h"
#define LED_PIN_FRONT 13
#define LED_PIN_BACK 12
#define LED_COUNT  20
#define BRIGHTNESS 10

//global
Adafruit_NeoPixel frontStrip(LED_COUNT, LED_PIN_FRONT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel backStrip(LED_COUNT, LED_PIN_BACK, NEO_GRB + NEO_KHZ800);

WiFiManager wm;
AsyncWebServer server(80);
WebSocketsServer webSocket(81);


int GetIdFromURL(AsyncWebServerRequest *request, String root)
{
  String string_id = request->url();
  string_id.replace(root, "");
  int id = string_id.toInt();
  return id;
}

String GetBodyContent(uint8_t *data, size_t len)
{
  String content = "";
  for (size_t i = 0; i < len; i++) {
    content .concat((char)data[i]);
  }
  return content;
}

void getData(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");
   // obtendríamos datos de GPIO, estado...
   StaticJsonDocument<300> jsonDoc;
   jsonDoc["id"] = random(0,10);
   jsonDoc["status"] = random(0,2);
   serializeJson(jsonDoc, *response);
   request->send(response);
}

 void setData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String bodyContent = GetBodyContent(data, len);
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, bodyContent);
  if (error) { request->send(400); return;}
 
  int rf = doc["rf"];
  int gf = doc["gf"];
  int bf = doc["bf"];
  int brightf = doc["brightf"];
  int rb = doc["rb"];
  int gb = doc["gb"];
  int bb = doc["bb"];
  int brightb = doc["brightb"];

ledStripFront(rf, gf, bf, brightf);
ledStripBack(rb, gb, bb, brightb);
 
  request->send(200);
}


void ledStripFront(byte R, byte G, byte B, byte brightness)
{
  Serial.print("Front RGB  R = ");
  Serial.print(R);
  Serial.print(" G = ");
  Serial.print(G);
  Serial.print(" B = ");
  Serial.print(B);
  Serial.print(" Brillo = ");
  Serial.println(brightness);
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
  Serial.print("Back RGB  R = ");
  Serial.print(R);
  Serial.print(" G = ");
  Serial.print(G);
  Serial.print(" B = ");
  Serial.print(B);
  Serial.print(" Brillo = ");
  Serial.println(brightness);
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
  //reset saved settings *ONLY FOR DEBUGGING*
  //wm.resetSettings();

  //Try to connect WiFi, then create AP
  wm.autoConnect("ESP32", "12345678");

  //the library is blocking. Once connected, the program continues
  Serial.println("ESP32 is connected to Wi-Fi network");
}

void InitServer()
{
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.on("/values", HTTP_GET, getData);
  server.on("/values", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setData);
  
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(400, "text/plain", "Not found");
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
    frontStrip.setPixelColor(i, frontStrip.Color(0, 40, 10));
    backStrip.setPixelColor(i, backStrip.Color(255, 255, 255));
    delay(10);
  }
  frontStrip.show();
  backStrip.show();
}


void frontLEDS(int r, int g, int b, int pwm)
{
  frontStrip.setBrightness(pwm);
  for ( int i = 0; i < LED_COUNT; i++)
  {
    frontStrip.setPixelColor(i, frontStrip.Color(r, g, b));
    delay(10);
  }
  frontStrip.show();
}



void backLEDS(int r, int g, int b, int pwm)
{
  backStrip.setBrightness(pwm);
  for ( int i = 0; i < LED_COUNT; i++)
  {
    backStrip.setPixelColor(i, backStrip.Color(r, g, b));
    delay(10);
  }
  backStrip.show();
}





void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght)  // When a WebSocket message is received
{
  Serial.printf("webSocketEvent! num: %d lenght: %d", num, lenght);
  switch (type) {
    case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      //IPAddress ip = webSocket.remoteIP(num);
      //webSocket.sendTXT(num, "Connected");
      Serial.printf("[%u] Connected url: %s\n", num, payload);
      break;
    case WStype_TEXT:
      //    String response = ProcessRequest();
      //webSocket.sendTXT(num, response);
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == 'F')                                                     // we get RGB data
      {
        int r = payload[1];
        int g = payload[2];
        int b = payload[3];
        int pwm = payload[4];
        frontLEDS(r, g, b, pwm);
      }
      else if (payload[0] == 'B')
      {
        int r = payload[1];
        int g = payload[2];
        int b = payload[3];
        int pwm = payload[4];
        backLEDS(r, g, b, pwm);
      }
      break;
  }
}


void InitWebSocketsServer()// Start a WebSocket server
{
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}



void setup(void)
{
  Serial.begin(115200);
  SPIFFS.begin();
  InitLEDs();
  InitWifi();
  InitServer();
  InitWebSocketsServer();
}

void loop(void)
{
/*  webSocket.loop();
  String message = GetFront(20, 40, 54, 12);
  webSocket.broadcastTXT(message);
*/
}
