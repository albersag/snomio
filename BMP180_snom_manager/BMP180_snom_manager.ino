/*
  The MIT License (MIT)
  Copyright (c) 2016 HOLGER IMBERY, CONTACT@CONNECTEDOBJECTS.CLOUD
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>


#define hostname_switch "bmp180"

bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

char SSID[40];
char SSID_PASSWORD[40];

const int LED = 16; //GPI16 - D0

Adafruit_BMP085 bmp;

ESP8266WebServer server(80);

String ipaddress = WiFi.localIP().toString();

void buildXMLtemp(){

  String ipaddress = WiFi.localIP().toString();

  String message = "<?xml version='1.0' encoding='UTF-8'?>";

   message+="<SnomIPPhoneText track='no'>";
   message+="<Title>Alicante</Title>";
   message+="<LocationX>55</LocationX>";
   message+="<LocationY>32</LocationY>";
   message+="<Text>";
   message+=bmp.readTemperature();
   message+=" ºC</Text>";
   message+="<Image>";
   message+="<LocationX>2</LocationX>";
   message+="<LocationY>23</LocationY>";
   message+="<Url>http://www.tommytape.com/wp-content/assets/icons/red-icon-temp.png</Url>";
   message+="</Image>";
   message+="<SoftKeyItem>";
   message+="<Name>F1</Name>";
   message+="<Label>Reload</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/temperature</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>F2</Name>";
   message+="<Label>Pressure</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/pressure</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>F3</Name>";
   message+="<Label>Altitude</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/altitude</URL>";
   message+="</SoftKeyItem>";
   message+="</SnomIPPhoneText>";
  
   server.send ( 200, "text/xml", message );
}

void buildXMLpress(){

  String ipaddress = WiFi.localIP().toString();
 
  String message = "<?xml version='1.0' encoding='UTF-8'?>";

   message+="<SnomIPPhoneText track='no'>";
   message+="<Title>Alicante</Title>";
   message+="<LocationX>55</LocationX>";
   message+="<LocationY>32</LocationY>";
   message+="<Text>";
   message+=bmp.readPressure();
   message+=" Pa </Text>";
   message+="<Image>";
   message+="<LocationX>2</LocationX>";
   message+="<LocationY>23</LocationY>";
   message+="<Url>http://www.omkarsupra.com/images/icons/high-pressure.png</Url>";
   message+="</Image>";
   message+="<SoftKeyItem>";
   message+="<Name>F1</Name>";
   message+="<Label>Reload</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/pressure</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>F2</Name>";
   message+="<Label>Temperature</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/temperature</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>F3</Name>";
   message+="<Label>Altitude</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/altitude</URL>";
   message+="</SoftKeyItem>";
   message+="</SnomIPPhoneText>";
   server.send ( 200, "text/xml", message );
}

void buildXMLalt(){

  String ipaddress = WiFi.localIP().toString();
 
  String message = "<?xml version='1.0' encoding='UTF-8'?>";

   message+="<SnomIPPhoneText track='no'>";
   message+="<Title>Alicante</Title>";
   message+="<LocationX>55</LocationX>";
   message+="<LocationY>32</LocationY>";
   message+="<Text>";
   message+=bmp.readAltitude();
   message+=" meters </Text>";
   message+="<Image>";
   message+="<LocationX>2</LocationX>";
   message+="<LocationY>23</LocationY>";
   message+="<Url>http://img.stockfresh.com/files/v/vtorous/x/50/918994_59503415.jpg</Url>";
   message+="</Image>";
   message+="<SoftKeyItem>";
   message+="<Name>F1</Name>";
   message+="<Label>Reload</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/altitude</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>F2</Name>";
   message+="<Label>Temperature</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/temperature</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>F3</Name>";
   message+="<Label>Pressure</Label>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/pressure</URL>";
   message+="</SoftKeyItem>";
   message+="</SnomIPPhoneText>";
   server.send ( 200, "text/xml", message );
}

void info() {
 server.send(200, "text/plain", "Hola mundo!");
}

void no_encontrado() {
 server.send(404,"text/plain","Error");
}
void setup() {
 //inicializa el puerto serie
 Serial.begin(9600);

  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 or BMP085 sensor at 0x77");
    while (1) {}
  }
 delay(10);
 //inicializa el led
 pinMode(LED,OUTPUT);
 digitalWrite(LED,LOW);

 WiFiManager wifiManager;

 wifiManager.setSaveConfigCallback(saveConfigCallback);

  //wifiManager.resetSettings();

 if (!wifiManager.autoConnect("BMP180_Snomio")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

 Serial.println(timestamp() + "  connected...");

 if (!MDNS.begin(hostname_switch)) {
    Serial.println(timestamp() + "  Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println(timestamp() + "  mDNS responder started");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

 
 //definimos los paths
 server.on ("/temperature",buildXMLtemp);
 server.on ("/pressure",buildXMLpress);
 server.on ("/altitude",buildXMLalt);
 server.onNotFound(no_encontrado);
 //inicializa el servidor web
 server.begin();
 Serial.println("Servidor HTTP activo");

 

}

void loop() {
 server.handleClient();
}

String timestamp() {
  char stamp[10];
  int currenth = millis() / 3600000;
  int currentm = millis() / 60000 - currenth * 60;
  int currents = millis() / 1000 - currenth * 3600 - currentm * 60;
  sprintf (stamp, "%03d:%02d:%02d", currenth, currentm, currents);
  return stamp;
}
