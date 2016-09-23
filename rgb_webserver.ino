#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#define PIN            D2

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);

const char* ssid="SSID";
const char* password="PASSWORD";
String webSite,javaScript,XML;


void buildWebsite(){
  webSite="<!DOCTYPE HTML>\n";
  webSite+="<BODY>\n";
  webSite+="<BR>Webserver SNOM Working! ...<BR>\n";
  webSite+="</BODY>\n";
  webSite+="</HTML>\n";
}


void buildXML(){
  XML="<?xml version='1.0'?>";
  XML+="<response>";
  XML+="</response>";
}


void handleWebsite(){
  buildWebsite();
  server.send(200,"text/html",webSite);
}

void handleXML(){
  buildXML();
  server.send(200,"text/xml",XML);
}

void handlellamada(){
  buildWebsite();
  server.send(200,"text/html",webSite);
  
  for (int veces = 0; veces < 10; veces++) {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {

        pixels.setPixelColor(0, pixels.Color(i * 255, j * 255, k * 255)); // Moderately bright green color.

        pixels.show(); // This sends the updated pixel color to the hardware.

        delay(100); // Delay for a period of time (in milliseconds).
      }
    }
  }
}
pixels.setPixelColor(0, pixels.Color(0, 0, 0 )); 
pixels.show();
}

void setup() {
  Serial.begin(115200);  
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED)delay(500);
  WiFi.mode(WIFI_STA);
  Serial.println("\n\nBOOTING ESP8266 ...");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Station IP address: ");
  Serial.println(WiFi.localIP());
  pixels.begin();
  server.on("/",handleWebsite);
  server.on("/xml",handleXML);
  server.on("/llamada",handlellamada);
  
  server.begin();  
}

void loop() {
  server.handleClient();
} 
