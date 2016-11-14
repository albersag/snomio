
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

#define THINGER_SERVER "iot2.thinger.io"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>


#define DHTPIN D4     // GPIO 4 pin of ESP8266
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define RELAYPIN D1

char SSID[40];
char SSID_PASSWORD[40];

#define hostname_switch "dht_relay"

DHT dht(DHTPIN, DHTTYPE); 

ESP8266WebServer server(80);

bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}



String ipaddress = WiFi.localIP().toString();
float humidity, temperature; 
unsigned long previousMillis = 0;        
const long interval = 2000; 
int status ;
String state;

void buildXMLtemp(){

  String ipaddress = WiFi.localIP().toString();

  String message = "<?xml version='1.0' encoding='UTF-8'?>";

  gettemperature();
  
   message+="<SnomIPPhoneMenu xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='../minibrowser_test.xsd'>";
   message+="<MenuItem name='Temperature: ";
   message+=temperature;
   message+="'>";
   message+="<Image>http://downloads.snom.com/tmp/snomio/voip2day/temperature.png</Image>";
   message+=" </MenuItem>";
   message+="<MenuItem name='Humidity: ";
   message+=humidity;
   message+="'>";
   message+="<Image>http://downloads.snom.com/tmp/snomio/voip2day/humidity.png</Image>";
   message+=" </MenuItem>";
   message+="</SnomIPPhoneMenu>";
   
   
  
   server.send ( 200, "text/xml", message );
}

void buildXMLrelay(){

  String ipaddress = WiFi.localIP().toString();

  gettemperature();

  getrelaystatus();

  String message = "<?xml version='1.0' encoding='UTF-8'?>";
  
   message+="<SnomIPPhoneMenu xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='../minibrowser_test.xsd'>";
   message+="<Menu name='Temperature & Humidity'>";
   message+="<Title>Voip2day Snom Workshop</Title>";
   message+="<MenuItem name='Temperature: ";
   message+=temperature;
   message+="'>";
   message+="<Image>http://downloads.snom.com/tmp/snomio/voip2day/temperature.png</Image>";
   message+=" </MenuItem>";
   message+="<MenuItem name='Humidity: ";
   message+=humidity;
   message+="'>";
   message+="<Image>http://downloads.snom.com/tmp/snomio/voip2day/humidity.png</Image>";
   message+=" </MenuItem>";
   message+="</Menu>";
   message+="<Menu name='Relay'>";
   message+="<Title>Voip2day Snom Workshop</Title>";
   message+="<MenuItem name='Relay Status: ";
   if (status == 0) {
   message+="OFF";
   }
   else {
   message+="ON";
   }
   message+="'>";
   message+="<Image>http://downloads.snom.com/tmp/snomio/voip2day/relay.png</Image>";
   message+=" </MenuItem>";
   message+="</Menu>";
   message+="<Menu name='Act Relay'>";
   message+="<Title>Voip2day Snom Workshop</Title>";
   message+="<MenuItem name='Set ON'>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/actrelay";
   message+="</URL>";
   message+="</MenuItem>";
   message+="<MenuItem name='Set OFF'>";
   message+="<URL>http://";
   message+=ipaddress;
   message+="/actrelay";
   message+="</URL>";
   message+=" </MenuItem>";
   message+="</Menu>";

   
   message+="</SnomIPPhoneMenu>";
   
   
  
   server.send ( 200, "text/xml", message );
}

void no_encontrado() {
 server.send(404,"text/plain","Error");
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAYPIN, OUTPUT);

  dht.begin();
  
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("ESP8266 Connecting to ");
  Serial.println(SSID);
  

 WiFiManager wifiManager;

 wifiManager.setSaveConfigCallback(saveConfigCallback);

  //wifiManager.resetSettings();

 if (!wifiManager.autoConnect("DHT11_Snomio")) {
    Serial.println("failed to connect and hit timeout");
    delay(1000);
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
 

server.on ("/temperature",buildXMLtemp);
server.on ("/relay",buildXMLrelay);
server.on ("/actrelay",actrelay);
server.begin();
 Serial.println("Servidor HTTP activo");

 
}

void gettemperature() {
  
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
   
    previousMillis = currentMillis;   

    humidity = dht.readHumidity();          // Read humidity (percent)
    temperature = dht.readTemperature();     // Read temperature as Fahrenheit
   
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}

void getrelaystatus(){
   
   status = digitalRead(RELAYPIN);
    
}

void actrelay(){
  
     
   status = digitalRead(RELAYPIN);

   if (status == 0) {
  digitalWrite(RELAYPIN, HIGH); 
  state="ON";
   }
   else {
   digitalWrite(RELAYPIN, LOW); 
   state="OFF";
   }

   String message = "<?xml version='1.0' encoding='UTF-8'?>";
  
   message+="<SnomIPPhoneMenu xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='../minibrowser_test.xsd'>";
   message+="<Menu name='Relay is set to:  ";
   message+=state;
   message+="'>";
   message+="<Title>Voip2day Snom Workshop</Title>";
   message+="</Menu>";   
   message+="</SnomIPPhoneMenu>";
   
   
  
   server.send ( 200, "text/xml", message );
    
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
