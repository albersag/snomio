
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
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ThingerESP8266.h>
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>

#define DBG_OUTPUT_PORT Serial

#define hostname_switch "crystal_car"



bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

const short int BUILTIN_LED2 = 16;  //GPIO16

char SSID[40];
char SSID_PASSWORD[40];


int motorASpeed = 1023;
int motorBSpeed = 1023;
int motorAForward = 1;
int motorBForward = 1;

ESP8266WebServer server(80);

String ipaddress = WiFi.localIP().toString();

void handleXMLCar() {
  
 String message = "<?xml version='1.0' encoding='UTF-8'?>";

   message+="<SnomIPPhoneImageFile track='no'>";
   message+="<LocationX>00</LocationX>";
   message+="<LocationY>00</LocationY>";
   message+="<SoftKeyItem>";
   message+="<Name>6</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
   message+="/engines?left=0&amp;right=1000</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>4</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
   message+="/engines?left=1000&amp;right=0</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>2</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
   message+="/engines?left=+1000&amp;right=+1000</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>8</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
   message+="/engines?left=-1000&amp;right=-1000</URL>";
   message+="</SoftKeyItem>";
   message+="<URL>http://10.0.200.92:8080/cam_pic.php</URL>";
   message+="<fetch mil='300'>http://"; 
   message+=ipaddress;
   message+="/car.xml</fetch>";
   message+="</SnomIPPhoneImageFile>";

   server.send ( 200, "text/xml", message );
    digitalWrite(BUILTIN_LED2, LOW);
  delay(500); // ms
  digitalWrite(BUILTIN_LED2, HIGH);
  delay(500); 
}

void handleMotor() {
  String leftText = server.arg(0);
  String rightText = server.arg(1);
  DBG_OUTPUT_PORT.println("[" + leftText + "][" + rightText + "]");
  int left = leftText.toInt();
  int right = rightText.toInt();
  if (left < 0) {
    motorAForward = 0;
  } else {
    motorAForward = 1;
  }
  if (right < 0) {
    motorBForward = 0;
  } else {
    motorBForward = 1;
  }
  analogWrite(5, abs(left));      // was left
  analogWrite(4, abs(right));     // was right
  digitalWrite(0, motorBForward); // was A
  digitalWrite(2, motorAForward); // was B

  delay(1000);  
  
  analogWrite(5, 0);
  analogWrite(4, 0);
  digitalWrite(0, 1);
  digitalWrite(2, 1);

  String message = "<?xml version='1.0' encoding='UTF-8'?>";

   message+="<SnomIPPhoneImageFile track='no'>";
   message+="<LocationX>00</LocationX>";
   message+="<LocationY>00</LocationY>";
   message+="<SoftKeyItem>";
   message+="<Name>6</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
   message+="/engines?left=0&amp;right=1000</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>4</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
    message+="/engines?left=1000&amp;right=0</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>2</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
   message+="/engines?left=+1000&amp;right=+1000</URL>";
   message+="</SoftKeyItem>";
   message+="<SoftKeyItem>";
   message+="<Name>8</Name>";
   message+="<URL track='no'>http://";
   message+=ipaddress;
   message+="/engines?left=-1000&amp;right=-1000</URL>";
   message+="</SoftKeyItem>";
   message+="<URL>http://10.0.200.92:8080/cam_pic.php</URL>";
   message+="<fetch mil='300'>http://";
   message+=ipaddress;
   message+="/car.xml</fetch>";
   message+="</SnomIPPhoneImageFile>";


  server.send ( 200, "text/xml", message );
  digitalWrite(BUILTIN_LED2, LOW);
  delay(500); // ms
  digitalWrite(BUILTIN_LED2, HIGH);
  delay(500); 
}

void setup() {
  
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  /// LED_user setup and test, for debug purpose.
  pinMode(BUILTIN_LED2, OUTPUT);
  digitalWrite(BUILTIN_LED2, LOW);
  delay(100); // ms
  digitalWrite(BUILTIN_LED2, HIGH);
  delay(300); // ms, pause because of AP mode

  // prepare GPIO2
  
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  // Connect to WiFi network

  WiFiManagerParameter custom_text("<p>SSID:</p>");   
  WiFiManagerParameter custom_ssid("ssid","SSID for thinger", SSID, 40);
  WiFiManagerParameter custom_text2("<p>SSID_PASSWORD: </p>");  
  WiFiManagerParameter custom_password("password","SSID_PASSWORD", SSID_PASSWORD, 40);

 WiFiManager wifiManager;

 wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_text);
  wifiManager.addParameter(&custom_ssid);
  wifiManager.addParameter(&custom_text2); 
  wifiManager.addParameter(&custom_password);

  //wifiManager.resetSettings();
 

 if (!wifiManager.autoConnect("CrystalCar_Snomio")) {
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

 
 ipaddress = WiFi.localIP().toString();
  
  server.on ( "/car.xml", handleXMLCar );
  server.on ( "/engines", handleMotor );

  //--------------- Start the server
  server.begin();
  
  DBG_OUTPUT_PORT.println("Server started");

  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);

  digitalWrite(5, 0);
  digitalWrite(4, 0);

  digitalWrite(0, 1);
  digitalWrite(2, 1);

 
  // show READY for use
  
  digitalWrite(BUILTIN_LED2, LOW);
  delay(300); // ms
  digitalWrite(BUILTIN_LED2, HIGH);

  

 
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
