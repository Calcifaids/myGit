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

void txSetup();
bool checkTxMutex();
void addToBuffer(uint8_t operation);
void shiftOffBuffer();

void txBegin();
void preambleISR();
void txBitTime();
void txWaitTime();
void txStopBit();
void volumeUp();
void volumeDown();
void startPreamble();
void powerOnDelay();
void delayThenHandle();

#endif
