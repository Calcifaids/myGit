#include "config.h"
#include "login.h"

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Setting up Web Server"));
  setupAP();
}

void loop(){
  ArduinoOTA.handle();
  server.handleClient(); 
}
