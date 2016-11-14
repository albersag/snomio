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
#include <Wire.h>
#include <Time.h>       // Time Manipulation
#include <WiFiUDP.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <DNSServer.h>            
#include <WiFiManager.h>        
#include <ESP8266mDNS.h>
#include <FS.h>

#define I2C_ADDRESS 0x3C

#define SCLPIN  5
#define SDAPIN  4

#define INCLUDE_SCROLLING 1

char SSID[40];
char SSID_PASSWORD[40];

#define hostname_switch "nokia_display"
  

// pin(14) - Serial clock out (SCLK)
// pin(13) - Serial data out (DIN)
// pin 12 - Data/Command select (D/C)
// pin 5 - LCD chip select (CS)
// pin 4 - LCD reset (RST)

const int LCDSelect = 4;

Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);


ESP8266WebServer server(80);  


WiFiClient client;

WiFiUDP           UDP; 

char timebuf[10];       // Time 
char cost[10];               
long eur = 0;
long cents = 0;               
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;
int startcounter=0;
unsigned long interval = 1000; 
int bill=0;
char user[]="108";
char localport[] ="8888"; 
char snomip[] = "10.0.1.37";
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

String message = "MESSAGE sip:user@remotephone:remoteport;transport=udp SIP/2.0\r\n"
                 "Via: SIP/2.0/UDP ipaddress:localport;branch=z9hG001b84f4bK6;rport\r\n"
                 "From: <sip:user@ipaddress:localport>;tag=as2e9533fad1\r\n"
                 "To: <sip:user@remotephone:remoteport;line=kutixubf>\r\n"
                 "Call-ID: 56784\r\n"
                 "CSeq: cs MESSAGE\r\n"
                 "Max-Forwards: 70\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: length\r\n"
                 "\r\n"
                 "Thanks for choosing our service\n"
                 "Last call cost was: cost1 \n"
                 "Please go to reception and pay";
                 
 void MessageUDP (IPAddress phone) {
   
   String messagetemp = message;
   String ipaddress = WiFi.localIP().toString();
   messagetemp.replace("user",user);
   messagetemp.replace("remotephone",phone.toString());
   messagetemp.replace("remoteport","5060");
   messagetemp.replace("localport","8888");
   messagetemp.replace("ipaddress",ipaddress);
   cseq=cseq+1;
   messagetemp.replace("cs",String(cseq));
   messagetemp.replace("length","93");
//   messagetemp.replace("cost1","1.04");
   messagetemp.replace("cost1",String(cost));

   
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
              
void lcd_display() {

 if (startcounter == 1) 
 {  

  currentMillis=millis();
  
  if(currentMillis - previousMillis > interval) {
    
  seconds=seconds + 1;
 
  if (seconds == 60) {
  minutes = minutes + 1;
  seconds=0;
  
  }

 if (minutes == 60){ 
  hours = hours + 1;
  minutes = 0;
  seconds=0;
  }


 if (cents==100){
  cents=0;
  eur=eur+1;
 
 }

  display.clearDisplay(); 
  display.setTextSize(1);
  display.setCursor(0,0);  
  display.println("Time:\n");
  sprintf(timebuf, "%02d:%02d:%02d\n",hours, minutes, seconds);             
  display.println(timebuf); 
  sprintf(cost, "Cost: %01d.%02d\n",eur, cents);             
  display.println(cost);
  display.display();
  cents+=1;
  previousMillis = currentMillis;  
  
  bill=1;
  
 }

 
  }
  else { 
 //   delay(5000);
    
    if (bill == 1) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Please\n");
    display.println("pay \n");
    sprintf(cost, "%01d.%02d euros",eur, cents);  
    display.println(cost);
    display.display();
    MessageUDP(phone);
    cents=0;
    eur=0;
    seconds=0;
    minutes=0;
    hours=0; 
    delay(5000);
    bill=0;
    }
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("Welcome\n");
    display.println("to our\n");
    display.println("CallShop\n");
    display.display();
  }
}

void setup(void) {
                               
  Serial.begin(9600);     
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
                        
  Wire.begin(SDAPIN, SCLPIN);         
  delay(50);
  display.begin();
  display.setContrast(55);
  display.clearDisplay();
  //display.drawBitmap(0, 0,  snom, 84, 48, 1);
  display.setTextSize(1);
  display.setCursor(30,20);
  display.println("SNOM");
  display.display();

  
  
  //Inicializa el módulo wifi


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

 if (!wifiManager.autoConnect("NokiaCallshop_Snomio")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

 Serial.println(timestamp() + "  connected...");

 
  strcpy(snomip, custom_snomip.getValue());
  
  strcpy(localport, custom_localport.getValue());

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
 
 
  server.on("/startstop", counter);   
                 
  server.begin(); 
  Serial.println("Servidor HTTP activo");                  
  UDP.begin(8888);
  
  thing["start_lcd"] = [](){

    counter();
   
    };

  thing["aoc"] = [](){

    MessageUDP(phone);
   
    };
    
          
  }


void loop(void) {
  server.handleClient();
  lcd_display();
  ReceiveUDP();    

//  delay(200);
            
}

void counter() {

    startcounter = !startcounter;
    if ( startcounter ==1){
    server.send(200,"text/html","START");
    }
    else  {
      server.send(200,"text/html","STOP");      

    }
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

