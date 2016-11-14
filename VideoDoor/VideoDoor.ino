

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
#include <WiFiClient.h>
#include <WiFiUDP.h>
#include <Adafruit_NeoPixel.h>
#include "Switch.h"
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>

#define PIN            D2

const byte buttonPin = D3;

const int ledPin = BUILTIN_LED;
int i=0;
#define hostname_switch "video_door"



Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
Switch button = Switch(buttonPin);

char SSID[40];
char SSID_PASSWORD[40];


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

IPAddress phone(10, 0, 1, 37);

WiFiClient client;

WiFiUDP           UDP; 
                   
int ledState = LOW;
int buttonState = LOW;
int thisButtonState = LOW;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 100;
unsigned int localport = 8888; 
char user[]="108";
char snomip[] = "10.0.1.37";
char videodoor[] = "10.0.1.32";
const int http_port = 80;
const char makecall[] = "GET /command.htm?number=122&outgoing_uri=104@10.0.1.3";
const char hangoutcall[] = "GET /command.htm?RELEASE_ALL_CALLS";
char         packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
char         ReplyBuffer[] = "????????????"; 
int cseq=1;

bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void switchonUDP (int status) {
   
   String messagetemp = message;
   String ipaddress = WiFi.localIP().toString();
   messagetemp.replace("user",user);
   messagetemp.replace("remotephone",snomip);
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

void call(int status) {
  
  // Attempt to make a connection to the remote server
  if ( !client.connect(videodoor, http_port) ) {
    Serial.print(F("Error Phone Connection\n"));
    Serial.println("Connection to: ");
    Serial.println(videodoor);


  }
 
  // Make an HTTP GET request
  if (status == 1)
  {
  client.println(makecall);
  }
  else{
  client.println(hangoutcall);
  }
  client.print("Host: ");
  client.println(videodoor);
  //client.print("Authorization: Basic c25vbTpzbm9tY2VydDEyMzQ=\r\n");
  client.println("Connection: close");
  client.println();

    
  

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

void setup() {
  Serial.begin(9600);    // Initialize serial communications
  Serial.println(F("Booting...."));

  pixels.begin(); 

 
 WiFiManager wifiManager;

 wifiManager.setSaveConfigCallback(saveConfigCallback);


  //wifiManager.resetSettings();

 if (!wifiManager.autoConnect("VideoDoor_Snomio")) {
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
 
 


  Serial.println(F("Ready!"));
  Serial.println(F("====================")); 

  UDP.begin(8888);
  
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);



}

void loop() {

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
        int green = !ledState * 255;
        pixels.setPixelColor(0, pixels.Color(0, green, 0));
        pixels.show();
        Serial.println(ledState);
        call(!ledState);
        switchonUDP(!ledState);
        
        
        
      }
      
      
      
    }
    
  }

  // persist for next loop iteration
  lastButtonState = thisButtonState;
}

String timestamp() {
  char stamp[10];
  int currenth = millis() / 3600000;
  int currentm = millis() / 60000 - currenth * 60;
  int currents = millis() / 1000 - currenth * 3600 - currentm * 60;
  sprintf (stamp, "%03d:%02d:%02d", currenth, currentm, currents);
  return stamp;
}

