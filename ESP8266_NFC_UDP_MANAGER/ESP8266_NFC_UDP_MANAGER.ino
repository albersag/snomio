

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

NFC Adafruit PN532 with Wemos D1

  HW setup:
  NFC   Wemos D1 (ESP):
  IRQ   D3    (GPIO0)
  RST   D4    (GPIO2)
  SCK   D5    (GPIO14)
  MISO  D6    (GPIO12)
  MOSI  D7    (GPIO13)
  SDA(NSS) D2 (GPIO4)

  
*/

#define THINGER_SERVER "iot2.thinger.io"

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFiClient.h>
//#include <EEPROM.h>
//#include <Base64.h>
#include <ThingerESP8266.h>
#include <WiFiUDP.h>
#include <PN532_SPI.h>
#include "PN532.h"
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>
#include <FS.h>

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

PN532_SPI pn532spi(SPI, D2);
PN532 nfc(pn532spi);

WiFiUDP           UDP;  

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

void unlock() {
  
  // Attempt to make a connection to the remote server
  if ( !client.connect(ssnomip, http_port) ) {
    Serial.print(F("Error Phone Conexion Card\n"));
     Serial.println("Conexión a : ");
    Serial.println(ssnomip);  
  }
  
  
  
  // Make an HTTP GET request

  client.println(urloff);
  client.print("Host: ");
  client.println(ssnomip);
  //client.print("Authorization: Basic c25vbTpzbm9tY2VydDEyMzQ=\r\n");
  client.println("Connection: close");
  client.println();

    
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
  delay(250);
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

  if (!wifiManager.autoConnect("NFC_Snomio")) {
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
    
    nfc.begin();

 uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  //nfc.setPassiveActivationRetries(0xFF);
  nfc.setPassiveActivationRetries(10);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A card");
  Serial.println("\n-----------\n");
    
}

void loop() { 
// Serial.println("Webserver ON");
   ReceiveUDP();

  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t card1[] = {0x4, 0x9E, 0x1D, 0x92, 0x17, 0x3C, 0x80};
  uint8_t card2[] = {0x4, 0x71, 0x59, 0x92, 0x17, 0x3C, 0x81};
  uint8_t yellow[] = {0x4, 0x7A, 0x6B, 0x92, 0x17, 0x3C, 0x81};
  uint8_t blue[] = {0x4, 0xC9, 0x91, 0x92, 0x17, 0x3C, 0x80};
  
  
  uint8_t uidLength;   // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++)
    {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX);
    }
    Serial.println("");

    // wait until the card is taken away
    while (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength)) yield(); //let ESPcore handle wifi stuff

    Serial.println("Seems to be a Mifare Ultralight tag (7 byte UID)");
    
    Serial.println("Reading page 4");
      
    uint8_t data[32];
      
    success = nfc.mifareultralight_ReadPage (4, data);
     
      if (success)
      {
        // Data seems to have been read ... spit it out
        nfc.PrintHexChar(data, 4);
        Serial.println(""); 
       }

    uint8_t matchingBytes=0;
    uint8_t matchingBytes2=0;
    uint8_t matchingBytes3=0;
    uint8_t matchingBytes4=0;
    
       for (uint8_t k=0; k < uidLength; k++)
         {
        if(uid[k]==card1[k]) matchingBytes++;
          }

      if (matchingBytes==7) //is the id matching with an intended length?
        {
       
          Serial.print("MATCH UNLOCK! ");
          unlock();
  
          }
       
       for (uint8_t k=0; k < uidLength; k++)
         {
        if(uid[k]==card2[k]) matchingBytes2++;
          }

        if (matchingBytes2==7) //is the id matching with an intended length?
        {
         Serial.print("MATCH LOCK! ");
          lock();
          }   
        
        for (uint8_t k=0; k < uidLength; k++)
         {
        if(uid[k]==blue[k]) matchingBytes3++;
          }

        if (matchingBytes3==7) //is the id matching with an intended length?
        {
         Serial.print("BLUE MATCH! ");
          
          }   

          
        for (uint8_t k=0; k < uidLength; k++)
         {
        if(uid[k]==yellow[k]) matchingBytes4++;
          }

        if (matchingBytes4==7) //is the id matching with an intended length?
        {
         Serial.print("YELLOW MATCH! ");
          
          }   

             
  }
  
  
  yield(); // PN532 probably timed out waiting for a card.. let's let the ESPcore handle wifi stuff

  
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

