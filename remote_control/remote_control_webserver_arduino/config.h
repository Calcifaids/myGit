#ifndef config_h
#define config_h

#include "FS.h"
#include "login.h"
#include "remote_control.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

extern ESP8266WebServer server;

void setupAP();
#endif
