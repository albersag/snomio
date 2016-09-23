/*
Many thanks to nikxha from the ESP8266 forum
*/

#include <ESP8266WiFi.h>
#include <SPI.h>
#include "MFRC522.h"
#include <WiFiClient.h>

/* wiring the MFRC522 to ESP8266 (ESP-12)
RST     = GPIO15
SDA(SS) = GPIO2 
MOSI    = GPIO13
MISO    = GPIO12
SCK     = GPIO14
GND     = GND
3.3V    = 3.3V
*/

#define RST_PIN	4 // RST-PIN für RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN	2  // SDA-PIN für RC522 - RFID - SPI - Modul GPIO2 

const char *ssid =	"SSID";	    // change according to your Network - cannot be longer than 32 characters!
const char *pass =	"PASSWORD";	// change according to your Network


const char snom_phone_ip[] = "10.0.1.31";
const int http_port = 80;
const char urlon[] = "/settings.htm?settings=save&keyboard_lock=on";
const char urloff[] = "/settings.htm?settings=save&keyboard_lock=off";

WiFiClient client;

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance

String get_uid(byte *buffer, byte bufferSize)
{
  String uid = "";
  for (byte j = 0; j < bufferSize; j++) {
    uid += String(buffer[j], HEX) + ",";
  }
  return uid;
}

void setup() {
  Serial.begin(9600);    // Initialize serial communications
  delay(250);
  Serial.println(F("Booting...."));
  
  SPI.begin();	         // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  
  WiFi.begin(ssid, pass);

  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 10)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("WiFi connected"));
  }
  
  Serial.println(F("Ready!"));
  Serial.println(F("====================")); 
  Serial.println(F("Snom RFID Unlock:"));
}

void loop() { 
  
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
    Serial.print(F("Tarjeta Valida"));
    getPage();
    Serial.println("Activado Lock");
    }
  else if (uid == "a,7d,13,bb,"){
    Serial.print(F("Tarjeta Invalida"));
    getPage2();
    Serial.println("Desactivado Lock");
   Serial.println();
    }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
}

bool getPage() {
  
  // Attempt to make a connection to the remote server
  if ( !client.connect(snom_phone_ip, http_port) ) {
    return false;
  }
  
  // Make an HTTP GET request
  client.println("GET /settings.htm?settings=save&keyboard_lock=on HTTP/1.1");
  client.print("Host: ");
  client.println(snom_phone_ip);
  client.println("Connection: close");
  client.println();
  
  return true;
}

bool getPage2() {
  
  // Attempt to make a connection to the remote server
  if ( !client.connect(snom_phone_ip, http_port) ) {
    return false;
  }
  
  // Make an HTTP GET request
  client.println("GET /settings.htm?settings=save&keyboard_lock=of HTTP/1.1");
  client.print("Host: ");
  client.println(snom_phone_ip);
  client.println("Connection: close");
  client.println();
  
  return true;
}
