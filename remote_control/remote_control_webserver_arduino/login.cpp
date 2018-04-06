#include "config.h"
#include "remote_control.h"
#include "login.h"
#include "FS.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

String cookieChars = "abcdefghijklmnopqrstuvwxyz0123456789", basicUsername = "user", basicPassword = "password", adminUsername = "admin", adminPassword = "admin";
String userCookie, adminCookie;

uint32_t timeoutTS = 0, timeoutThreshold = 300000;

//Generate cookies on start
void generateSession(){
  userCookie = generateCookie(userCookie);
  adminCookie = generateCookie(adminCookie);
  Serial.print("User Cookie = ");
  Serial.println(userCookie);
  Serial.print("Admin Cookie = ");
  Serial.println(adminCookie);
}

//Serve 404 page
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

//Check correct user cookie in place and direct to root
void handleRoot(){
  unsigned long timestamp = millis();
  Serial.println(timestamp);
  if (!checkUserAuth()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  //If Arg exists then not first call and should process button press
  if (server.hasArg("userResponse")){
    timeoutTS = millis();
    bool checkResult = checkToAddBuffer();
    if (checkResult == true){
      server.send(200, "text/html");
      String returnedValue = server.arg("userResponse");
      Serial.print("User response = ");
      Serial.println(returnedValue);
      uint8_t sendCommand = atoi(returnedValue.c_str());
      addToBuffer(sendCommand);
    }
    else{
      server.send(204, "text/html");
      Serial.println("System currently locked so cannot add to buffer.");
    }
    
  }
  else{
    //Serve remote_control page
    timeoutTS = millis();
    File remotePage = SPIFFS.open("/remote_control.html", "r");
    server.streamFile(remotePage, "text/html");
    remotePage.close();
    return;
  }
}

//Check correct admin cookie in place and direct to admin page
void handleAdmin(){
  //Redirect if tokens not in place
  if (!checkAdminAuth()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  //Set new password if correct Arg provided
  if (server.hasArg("newPassword")){
    String returnedValue = server.arg("newPassword");
    Serial.print("New user password = ");
    Serial.println(returnedValue);
    basicPassword = returnedValue;
    server.send(200, "text/html");
  }
  //Set new threshold if correct Arg provided
  else if (server.hasArg("newThreshold")){
    String returnedValue = server.arg("newThreshold");
    Serial.print("New threshold to set = ");
    Serial.println(returnedValue);
    uint8_t newThresh = atoi(returnedValue.c_str());
    bool successCheck = updateVolumeThreshold(newThresh);
    if (successCheck == true){
      server.send(200, "text/html");
    }
  }
  //Must be initial request, so serve page
  else{
    File adminPage = SPIFFS.open("/admin.html", "r");
    server.streamFile(adminPage, "text/html");
    adminPage.close();
    return;
  }
}

//Post login form and handle response
void handleLogin(){
  //Check if login info has been posted
  if (server.hasArg("user") && server.hasArg("pass")){
    //Check if basic user details correct
    if (server.arg("user") == basicUsername && server.arg("pass") == basicPassword){
      //Set user cookie and redirect to root
      timeoutTS = millis();
      server.sendHeader("Set-Cookie", "user=" + userCookie);
      server.sendHeader("Set-Cookie", "admin=0");
      server.sendHeader("Location","/");
      server.sendHeader("Cache-Control","no-cache");
      server.send(301);
      return;
    }
    //Check if admin user details correct
    else if (server.arg("user") == adminUsername && server.arg("pass") == adminPassword){
      //Set admin and user cookes and direct to Admin page
      timeoutTS = millis();
      server.sendHeader("Set-Cookie", "user=" + userCookie);
      server.sendHeader("Set-Cookie", "admin=" + adminCookie);
      server.sendHeader("Location","/admin");
      server.sendHeader("Cache-Control","no-cache");
      server.send(301);
      return;
    }
  }

  //Send login page
  File loginPage = SPIFFS.open("/login.html", "r");
  server.streamFile(loginPage, "text/html");
  loginPage.close();
}

//Set admin and user to 0 & redirect to login
void handleLogout(){
  server.sendHeader("Set-Cookie", "user=0");
  server.sendHeader("Set-Cookie", "admin=0");
  server.sendHeader("Location","/login");
  server.sendHeader("Cache-Control","no-cache");
  server.send(301);
  resetCookies();
}

//Itterate through alpha-num to generate 32 bit cookie
String generateCookie(String cookie){
  cookie = "";
  for( uint8_t i = 0; i < 32; i++) cookie += cookieChars[random(0, cookieChars.length())];
  return cookie;
}

//Check User Cookie
bool checkUserAuth(){
  if (server.hasHeader("Cookie")){
    String receivedCookie = server.header("Cookie");
    String compareCookie = "user=" + userCookie;
    if (receivedCookie.indexOf(compareCookie) != -1){
      return true;
    }
  }
  return false;
}

// Check Admin Cookie
bool checkAdminAuth(){
  if (server.hasHeader("Cookie")){
    String receivedCookie = server.header("Cookie");
    String compareCookie = "admin=" + adminCookie;
    if (receivedCookie.indexOf(compareCookie) != -1){
      return true;
    }
  }
  return false;
}

void checkTimeout(){
  //If timeout has occured reset session
  if (millis() > timeoutTS + timeoutThreshold){
    resetCookies();
  }
}

//Gen new cookies & reset timeout
void resetCookies(){
  generateSession();
  timeoutTS = millis();
}

