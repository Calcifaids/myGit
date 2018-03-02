#ifndef remote_control_h
#define remote_control_h

#include "FS.h"
#include "config.h"
#include "login.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include "sigma_delta.h"

void tx_Setup();
void tx_Begin();
void preambleISR();
void txBitTime();
void txWaitTime();
void txStopBit();

#endif
