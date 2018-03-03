#include "config.h"
#include "login.h"
#include "FS.h"
#include "remote_control.h"

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
  //Check Mutex is free and begin tx
  bool checkResult = checkTxMutex();
  if (checkResult == true){
    txBegin();
  }
}
