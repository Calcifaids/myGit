#include "config.h"
#include "login.h"
#include "FS.h"

void setup(){
  Serial.begin(115200);
  SPIFFS.begin();
  Serial.println();
  Serial.println(F("Setting up Web Server"));
  setupAP();
}

void loop(){
  ArduinoOTA.handle();
  server.handleClient(); 
}
