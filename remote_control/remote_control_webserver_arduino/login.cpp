#include "config.h"
#include "remote_control.h"
#include "login.h"
#include "FS.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

/*
 * Need to Add:
 * - Regeneration of cookies on the fly?
 * - Regenerate cookies after set period of inactivity (preventing old cookies from being used)
 * - Better CSS
 * - Increase responsiveness of remote.html
 * - Auto Generate user Password 
 * - Add re-generate user password for admin
 * 
 */

/*Convert to a file and serve file?*/
const String loginPage = "<!DOCTYPE html><html><head><title>Login</title></head><body> <div id=\"login\"> <form action='/login' method='POST'> <center> <h1>Login </h1><p><input type='text' name='user' placeholder='User name'></p><p><input type='password' name='pass' placeholder='Password'></p><br><button type='submit' name='submit'>login</button></center> </form></body></html>";

String cookieChars = "abcdefghijklmnopqrstuvwxyz0123456789", basicUsername = "user", basicPassword = "password", adminUsername = "admin", adminPassword = "admin";
String userCookie, adminCookie;

uint32_t timeoutTS = 0, timeoutThreshold = 300000;



void writeToLog(){
  File accessLog = SPIFFS.open ("/accessLog.txt", "r");
  
  //Generate files if not present
  //Write new line to temporary file
  //append original across to temporary file
  //Look for EOF (If reach line 101 then delete and make EOF)
  //Delete original
  //Rename Copy to original
  
  /*
  File remotePage = SPIFFS.open("/remote_control.html", "r");
    server.streamFile(remotePage, "text/html");
    remotePage.close();
  */
}

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
  if(server.hasArg("userResponse")){
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
  if(!checkAdminAuth()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  /*!ADD HANDLING AND INSERT UPDATE TIMESTAMP!*/
  if (server.hasArg("newPassword")){
    String returnedValue = server.arg("newPassword");
    Serial.print("New user password = ");
    Serial.println(returnedValue);
    basicPassword = returnedValue;
    server.send(200, "text/html");
  }
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
  else{
    
    File adminPage = SPIFFS.open("/admin.html", "r");
    server.streamFile(adminPage, "text/html");
    adminPage.close();
    return;
  }
  //server.send(200, "text/html", "You reached the Admin Page<cr>Click <a href='/'>here</a> to access the front end remote.<cr>  <a href='/logout'>Logout</a>");
}

//Post login form and handle response
void handleLogin(){
  //Check if login info has been posted
  if(server.hasArg("user") && server.hasArg("pass")){
    //Check if basic user details correct
    if(server.arg("user") == basicUsername && server.arg("pass") == basicPassword){
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
    else if(server.arg("user") == adminUsername && server.arg("pass") == adminPassword){
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

  //Send login page here
  server.send(200, "text/html", loginPage);
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

/*Make more complex to handle both use and admin auth?*/
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
  if(millis() > timeoutTS + timeoutThreshold){
    resetCookies();
  }
}

void resetCookies(){
  generateSession();
  timeoutTS = millis();
}

