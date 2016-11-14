
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

#include <FS.h>
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <WiFiUDP.h>
#include <Adafruit_NeoPixel.h>
#include "Switch.h"
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>  

ESP8266WebServer server(80);

#define PIN            D2

//const int buttonPin = D3;
const byte buttonPin = D3;

const int ledPin = BUILTIN_LED;
int i=0;

#define hostname_switch "green_switch"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
Switch button = Switch(buttonPin);

String message = "MESSAGE sip:user@remotephone:remoteport;transport=udp SIP/2.0\r\n"
                   "Via: SIP/2.0/UDP ipaddress:localport;branch=z9hG001b84f4bK6;rport\r\n"
                   "From: <sip:user@ipaddress:localport>;tag=as2e9533fad1\r\n"
                   "To: <sip:user@remotephone:remoteport;line=kutixubf>\r\n"
                   "Call-ID: 56784\r\n"
                   "CSeq: cs MESSAGE\r\n"
                   "Max-Forwards: 70\r\n"
                   "Content-Type: application/x-buttons\r\n"
                   "\r\n"
                   "k=id\r\n"
                   "o=colour\r\n"
                   "c=value\r\n";


//WiFiClient client;

WiFiUDP           UDP; 
                   
int ledState = LOW;
int buttonState = LOW;
int thisButtonState = LOW;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 100;
char localport[] = "8888"; 
char user[]="108";
char snomip[]="10.0.1.37";
char         packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
char         ReplyBuffer[] = "????????????"; 
int cseq=1;


unsigned int dir0=atoi(strtok(snomip, "."));
unsigned int dir1=atoi(strtok(NULL, "."));
unsigned int dir2=atoi(strtok(NULL, "."));
unsigned int dir3=atoi(strtok(NULL, "\0"));
  
IPAddress phone (dir0,dir1,dir2,dir3);


bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

String timestamp() {
  char stamp[10];
  int currenth = millis() / 3600000;
  int currentm = millis() / 60000 - currenth * 60;
  int currents = millis() / 1000 - currenth * 3600 - currentm * 60;
  sprintf (stamp, "%03d:%02d:%02d", currenth, currentm, currents);
  return stamp;
}

void setup() {
  Serial.begin(9600);    // Initialize serial communications
  Serial.println(F("Booting...."));


  //SPIFFS.format();

  Serial.println(timestamp() + "  mounting FS...");

   if (SPIFFS.begin()) {
    Serial.println(timestamp() + "  mounted file system");
    if (SPIFFS.exists("/values.config")) {
      //file exists, reading and loading
      Serial.println(timestamp() + "  reading config file");
      File configFile = SPIFFS.open("/values.config", "r");
      if (configFile) {
        Serial.println(timestamp() + "  opened config file");
        String snomip=configFile.readStringUntil('\n');
        String localport=configFile.readStringUntil('\n');

        Serial.println("Se ha leido:\n");
        Serial.println(snomip);
        Serial.println(localport);
         }
        
      }
   }




//  WiFi.begin(ssid, pass);
 
 pixels.begin(); 

  UDP.begin(8888);
  
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  WiFiManagerParameter custom_text("<p>Phone IP Address:</p>");   
  WiFiManagerParameter custom_snomip("snomipaddress","Phone IP Address", snomip, 40);
  WiFiManagerParameter custom_text2("<p>SIP Local Port: </p>");  
  WiFiManagerParameter custom_localport("snomlocalport","SIP Local Port", localport, 40);

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_text);
  wifiManager.addParameter(&custom_snomip);
  wifiManager.addParameter(&custom_text2); 
  wifiManager.addParameter(&custom_localport);

  //wifiManager.resetSettings();

  if (!wifiManager.autoConnect("Switch_Snomio")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println(timestamp() + "  connected...");

  strcpy(snomip, custom_snomip.getValue());
  
  strcpy(localport, custom_localport.getValue());

//  shouldSaveConfig = true;

  if (shouldSaveConfig) {
    Serial.println(timestamp() + "  saving config");

    File configFile = SPIFFS.open("/values.config", "w");
    if (!configFile) {
      Serial.println(timestamp() + "  failed to open config file for writing");
    }

    configFile.println(phone.toString());
    configFile.println(localport);
    configFile.close();

  }


  if (!MDNS.begin(hostname_switch)) {
    Serial.println(timestamp() + "  Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println(timestamp() + "  mDNS responder started");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  


}

void loop2() {

  ReceiveUDP();

   
  thisButtonState = digitalRead(buttonPin);

  // if the current state does not match the previous state
  // the button was just pressed/released, or is transition noise
  if (thisButtonState != lastButtonState) {
    // reset the timer
    lastDebounceTime = millis();
  }

  // once delay millis have elapsed, if the state remains the same, register the press
  if ((millis() - lastDebounceTime) > debounceDelay) {

    // if the button state has changed
    if (thisButtonState != buttonState) {
      buttonState = thisButtonState;

      // only toggle the LED if the buttonState has switched from LOW to HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        // toggle the LED
      digitalWrite(ledPin, ledState);
        int green = ledState * 255;
        pixels.setPixelColor(0, pixels.Color(0, green, 0));
        pixels.show();
        Serial.println(ledState);
        switchonUDP(!ledState,phone);
         
        
      }
      
      
      
    }
    
  }

  // persist for next loop iteration
  lastButtonState = thisButtonState;
}

void loop() {
  server.handleClient();
  button.poll();
  ReceiveUDP();

  
  if (button.switched()) {
     
  }

  if (button.pushed()) {
    
  pixels.setPixelColor(0, pixels.Color(i*255, 0, 0));
  pixels.show();
      i=!i;
  }

 // if (button.longPress()) {
    
 //   pixels.setPixelColor(0, pixels.Color(0, i*255, 0));
 //    pixels.show();
 //    i=!i;
    
 // }

//  if (button.doubleClick()) {
//    pixels.setPixelColor(0, pixels.Color(0, 0, i*255));
//     pixels.show();
//      i=!i;
//  }

  if (button.released()) {
 //   pixels.setPixelColor(0, pixels.Color(0, 0, 0));
 //    pixels.show();
  }
}


void switchonUDP (int status,IPAddress phone) {

   String messagetemp = message;
   String ipaddress = WiFi.localIP().toString();
   
   messagetemp.replace("user",user);
   messagetemp.replace("remotephone",phone.toString());
   messagetemp.replace("remoteport","5060");
   messagetemp.replace("localport","8888");
   messagetemp.replace("ipaddress",ipaddress);
   cseq=cseq+1;
   messagetemp.replace("cs",String(cseq));
   if (status == 1)
   {
   messagetemp.replace("value","on");
   }
   else
   {
    messagetemp.replace("value","off");
   }
   
   messagetemp.replace("id","1");
   messagetemp.replace("colour","green");
   
   char message2[messagetemp.length()+1];
   messagetemp.toCharArray(message2, messagetemp.length()+1);
      
   Serial.println("");
   Serial.print("Sent Packet of size ");
   Serial.println(sizeof(message2));
   Serial.println("Contents:");
   Serial.println(message2);

   UDP.beginPacket(phone,5060);
   UDP.write(message2);
   UDP.endPacket();
  
}

void ReceiveUDP()
{
 int packetSize = UDP.parsePacket();
if(packetSize)
{
Serial.println("");
Serial.print("Received packet of size ");
Serial.println(packetSize);
Serial.print("From ");
IPAddress remote = UDP.remoteIP();
for (int i =0; i < 4; i++)
{
Serial.print(remote[i], DEC);
if (i < 3)
{
Serial.print(".");
}
}
Serial.print(", port ");
Serial.println(UDP.remotePort());

// read the packet into packetBufffer
UDP.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
Serial.println("Contents:");
Serial.println(packetBuffer);
//int value = packetBuffer[0]*256 + packetBuffer[1];
//Serial.println(value);

// send a reply, to the IP address and port that sent us the packet we received
//UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
//UDP.write(ReplyBuffer);
//UDP.endPacket();
}
}


