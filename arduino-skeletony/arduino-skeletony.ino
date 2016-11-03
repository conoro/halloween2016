// Copyright 2016 Conor O'Neill <conor@conoroneill.com>
// Apache License Version 2.0. See LICENSE file

// Portions via http://www.esp8266.com/viewtopic.php?p=21933&sid=2fe36f346fd7979d558caa5cff55ae0c#p21933
// Portions via https://github.com/tzapu/WiFiManager

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"          // https://github.com/tzapu/WiFiManager

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

const int SKELETONY_PIN = 12; //D6 aka GPIO12 on NodeMCU. Connected to Pin D2 on the Arduino Nano.
const int USER_PIN = 4; // D2 aka GPIO4 on NodeMCU
 
const char* ssid = "SKELETONY";
const char* password = "damnedpassword";
ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
 
void setup(void){
  pinMode(SKELETONY_PIN, OUTPUT);
  pinMode(USER_PIN, INPUT_PULLUP);
  digitalWrite(SKELETONY_PIN, 0);
  Serial.begin(115200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  // If I'm pressing the User button 
  if (digitalRead(USER_PIN) == LOW){
    WiFi.disconnect();
    if (!wifiManager.startConfigPortal("SKELETONY-CONFIG", "damnedpassword")) {
       Serial.println("failed to connect and hit timeout");
       delay(3000);
       //reset and try again, or maybe put it to deep sleep
       ESP.reset();
       delay(5000);
     } else {
       Serial.println("Hopefully Saved and connected");      
     }
     
  } else {
    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if(!wifiManager.autoConnect("SKELETONY-CONFIG", "damnedpassword")) {
      Serial.println("failed to connect and hit timeout");
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(1000);
    } 
  }
  //if you get here you have connected to the WiFi
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("skeletony1", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  MDNS.addService("skeletony", "tcp", 80); // Announce skeletony service

  server.on("/", handleRoot);
  
  server.on("/on", [](){
  server.send(200, "text/plain", "Skeletony=ON");
  digitalWrite(SKELETONY_PIN, 1);
  delay(1000);
  digitalWrite(SKELETONY_PIN, 0);
  });
  
  server.on("/off", [](){
  server.send(200, "text/plain", "Skeletony=OFF");
  digitalWrite(SKELETONY_PIN, 0);
  });

  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void){
  server.handleClient();
}
