#ifndef login_h
#define login_h

#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

void generateSession();
void handleRoot();
void handleLogin();
void handleAdmin();
void handleLogout();
void handleNotFound();
String generateCookie(String);
bool checkUserAuth();
bool checkAdminAuth();

#endif
