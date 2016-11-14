
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
#include <SPI.h>
#include "MFRC522.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUDP.h>
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>

#include <FS.h>

/* wiring the MFRC522 to ESP8266 (ESP-12)
RST     = D2
SDA(SS) = D4 
MOSI    = D7
MISO    = D6
SCK     = D5
GND     = GND
3.3V    = 3.3V
*/

ESP8266WebServer server(80);


#define RST_PIN	4 // RST-PIN für RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN	2  // SDA-PIN für RC522 - RFID - SPI - Modul GPIO2


#define hostname_switch "rfid_reader"


char SSID[40];
char SSID_PASSWORD[40];

String st;
String content;

char snomip[] = "10.0.1.37";
char ssnomip[] = "10.0.1.37";
const int http_port = 80;
const char urlon[] = "GET /settings.htm?settings=save&keyboard_lock=on";
const char urloff[] = "GET /settings.htm?settings=save&keyboard_lock=off";
char inputString[] = "Base64EncodeExample";
int inputStringLength = sizeof(inputString);
int          ack_Count =    0;
char localport[] ="8888"; 
char user[]="108";

char         packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
char         ReplyBuffer[] = "????????????"; 

String ipaddress = WiFi.localIP().toString();

String message = "MESSAGE sip:user@remotephone:remoteport;transport=udp SIP/2.0\r\n"
                   "Via: SIP/2.0/UDP ipaddress:localport;branch=z9hG001b84f4bK6;rport\r\n"
                   "From: <sip:user@ipaddress:localport>;tag=as2e9533fad1\r\n"
                   "To: <sip:user@remotephone:remoteport;line=kutixubf>\r\n"
                   "Call-ID: 56784\r\n"
                   "CSeq: cs MESSAGE\r\n"
                   "Max-Forwards: 70\r\n"
                   "Event: snom-settings\r\n"
                   "Content-Type: application/xml\r\n"
                   "Content-Length: length\r\n"
                   "\r\n"
                   "<settings>\r\n"
                   "<phone-settings>\r\n"
                   "<keyboard_lock>lockvalue</keyboard_lock>\r\n"
                   "</phone-settings>\r\n"
                   "</settings>\r\n";

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

WiFiClient client;

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance

WiFiUDP           UDP;  



String get_uid(byte *buffer, byte bufferSize)
{
  String uid = "";
  for (byte j = 0; j < bufferSize; j++) {
    uid += String(buffer[j], HEX) + ",";
  }
  return uid;
}



bool lock() {
  
  // Attempt to make a connection to the remote server
  if ( !client.connect(ssnomip, http_port) ) {
    Serial.print(F("Error Phone Conexion Card\n"));
    Serial.println("Conexión a : ");
    Serial.println(ssnomip);
    return false;

  }
    
 
  // Make an HTTP GET request
  
  client.println(urlon);
  client.print("Host: ");
  client.println(ssnomip);
  //client.print("Authorization: Basic c25vbTpzbm9tY2VydDEyMzQ=\r\n");
  client.println("Connection: close");
  client.println();

    
  
  return true;
}

bool unlock() {
  
  // Attempt to make a connection to the remote server
  if ( !client.connect(ssnomip, http_port) ) {
    Serial.print(F("Error Phone Conexion Card\n"));
     Serial.println("Conexión a : ");
    Serial.println(ssnomip);  
    return false;
  }
  
  
  
  // Make an HTTP GET request

  client.println(urloff);
  client.print("Host: ");
  client.println(ssnomip);
  //client.print("Authorization: Basic c25vbTpzbm9tY2VydDEyMzQ=\r\n");
  client.println("Connection: close");
  client.println();

    
  
  return true;
}

void lockUDP (IPAddress phone) {
   
   String messagetemp = message;
   String ipaddress = WiFi.localIP().toString();
   messagetemp.replace("user",user);
   messagetemp.replace("remotephone",phone.toString());
   messagetemp.replace("remoteport","5060");
   messagetemp.replace("localport","8888");
   messagetemp.replace("ipaddress",ipaddress);
   int cseq=random(50,60);
   messagetemp.replace("cs",String(cseq));
   messagetemp.replace("lockvalue","on");
   messagetemp.replace("length","97");
   
   
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
void unlockUDP (IPAddress phone) {
                   
   String messagetemp = message;
   String ipaddress = WiFi.localIP().toString();
   messagetemp.replace("user",user);
   messagetemp.replace("remotephone",phone.toString());
   messagetemp.replace("remoteport","5060");
   messagetemp.replace("localport","8888");
   messagetemp.replace("ipaddress",ipaddress);
   int cseq=random(50,60);
   messagetemp.replace("cs",String(cseq));
   messagetemp.replace("lockvalue","off");
   messagetemp.replace("length","98");
   
   
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

  void setup() {
  Serial.begin(9600);    // Initialize serial communications
//  EEPROM.begin(512);
  delay(250);
  Serial.println(F("Booting...."));
  
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

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


  WiFiManagerParameter custom_text3("<p>Phone IP Address:</p>");   
  WiFiManagerParameter custom_snomip("snomipaddress","Phone IP Address", snomip, 40);
  WiFiManagerParameter custom_text4("<p>SIP Local Port: </p>");  
  WiFiManagerParameter custom_localport("snomlocalport","SIP Local Port", localport, 40);

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_text3); 
  wifiManager.addParameter(&custom_snomip);
  wifiManager.addParameter(&custom_text4);
  wifiManager.addParameter(&custom_localport);

  //wifiManager.resetSettings();

  if (!wifiManager.autoConnect("RC522_Snomio")) {
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
 
 
  
  Serial.println(F("Ready!"));
  Serial.println(F("====================")); 
  Serial.println(F("Snom RFID Unlock:"));

   UDP.begin(8888);
   

    
    
}

void loop() { 
// Serial.println("Webserver ON");
   ReceiveUDP();

//   server.handleClient();

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card)

  String uid = get_uid(mfrc522.uid.uidByte, mfrc522.uid.size);

  if (uid == "25,4,25,77,"){
    Serial.print(F("Valid Card\n"));
    lock();
    Serial.println("Lock activated\n");
    }
  else if (uid == "37,fb,e9,ee,"){
    Serial.print(F("Valid Card\n"));
    unlock();
    Serial.println("Lock Deactivated\n");
    }
    else 
    {
      Serial.print(F("Invalid Card\n"));
      Serial.println();
    }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  
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

String timestamp() {
  char stamp[10];
  int currenth = millis() / 3600000;
  int currentm = millis() / 60000 - currenth * 60;
  int currents = millis() / 1000 - currenth * 3600 - currentm * 60;
  sprintf (stamp, "%03d:%02d:%02d", currenth, currentm, currents);
  return stamp;
}
