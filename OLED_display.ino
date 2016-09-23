/*
ESP8266 D1 Mini OLED display with SH1106 example

Based on https://github.com/costonisp/ESP8266-I2C-OLED
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C

#define SCLPIN  5
#define SDAPIN  4

#define INCLUDE_SCROLLING 1

SSD1306AsciiWire oled;

#define SSID "SSID"                              // insert your SSID
#define PASS "PASSWORD"                              // insert your password
// ******************* String form to sent to the client-browser ************************************
String form =
  "<p>"
  "<center>"
  "<h1>Enter Text to Display</h1>"
  "<form action='msg'><p>Enter text: <input type='text' name='msg' size=50 autofocus> <input type='submit' value='Submit'></form>"
  "</center>";

ESP8266WebServer server(80);                             
 long period;

void handle_msg() {
  oled.clear();                        
 
  server.send(200, "text/html", form);    

  // Display msg on Oled
  String msg = server.arg("msg");
  Serial.println(msg);
  String decodedMsg = msg;
 
  decodedMsg.replace("+", " ");      
  decodedMsg.replace("%21", "!");  
  decodedMsg.replace("%22", "");  
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");  
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");  
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");  
  decodedMsg.replace("%2C", ",");  
  decodedMsg.replace("%2F", "/");   
  decodedMsg.replace("%3A", ":");    
  decodedMsg.replace("%3B", ";");  
  decodedMsg.replace("%3C", "<");  
  decodedMsg.replace("%3D", "=");  
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");  
  decodedMsg.replace("%40", "@"); 
  Serial.println(decodedMsg);                   
  unsigned int lengte = decodedMsg.length();     
  oled.println("LLAMADA DE\n");
  decodedMsg.remove(3,10);
  oled.println(decodedMsg);
  delay(10000);
  oled.clear();
  oled.println("SNOM\n");
  oled.println("REMOTE\n");
  oled.println("DISPLAY...\n");

}

void setup(void) {
//ESP.wdtDisable();                              
  Serial.begin(115200);                           
  Wire.begin(SDAPIN, SCLPIN);         
  delay(50);
  oled.begin(&MicroOLED64x48, I2C_ADDRESS);
  oled.setFont(lcd5x7);
  oled.set1X(); 
  oled.setScroll(true);
  WiFi.begin(SSID, PASS);                         
  while (WiFi.status() != WL_CONNECTED) {         
    delay(500);
    Serial.print(".");
  }
  
  server.on("/", []() {
    server.send(200, "text/html", form);
  });
  server.on("/msg", handle_msg);                  
  server.begin();                                
  oled.clear();
  
  Serial.print("SSID : ");                        
  Serial.println(SSID); 
   
  oled.print("SSID: ");
  oled.print(SSID);
 

  oled.println("\n\nIP\n");
  oled.println(WiFi.localIP()); 
  
  // OLED first message 
  Serial.println(WiFi.localIP());                 
  Serial.print(analogRead(A0));
  int test = 13;
  pinMode(test,OUTPUT);
  digitalWrite(test,HIGH);
  delay(1000);
  digitalWrite(test,LOW);
}


void loop(void) {
  server.handleClient();                       
}

