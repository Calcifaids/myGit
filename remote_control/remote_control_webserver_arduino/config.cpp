#include "config.h"
#include "remote_control.h"
#include "FS.h"
#include "login.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>


const char *ssid = "CbrainesRemote";
const char *password = "turnonthetv";

ESP8266WebServer server(80);

void setupAP(){
  //Setup as Access Point
  WiFi.softAP(ssid, password, 6);
  //Enable OTA
  ArduinoOTA.begin();

  //Print IP
  IPAddress apIP = WiFi.softAPIP();
  Serial.print(F("AP IP = "));
  Serial.println(apIP);

  //Generate cookies
  generateSession();

  //Apply client handlers
  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  /*Add in 301 redirets*/
  server.on("/admin", handleAdmin);
  server.on("/logout", handleLogout);  
  server.onNotFound(handleNotFound);

  //Collect User-Agent & Cookie in http header
  const char * headerKeys[] = {"User-Agent","Cookie"} ;
  size_t headerKeysSize = sizeof(headerKeys)/sizeof(char*);
  server.collectHeaders(headerKeys, headerKeysSize );

  txSetup();
  
  //Start HTTP server
  server.begin();
  Serial.println("HTTP Server Started");
}

