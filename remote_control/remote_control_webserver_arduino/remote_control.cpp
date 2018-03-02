#include "remote_control.h"
#include "config.h"
#include "login.h"
#include "FS.h"
#include "sigma_delta.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>



/*
 * ISR TIMINGS
 * prescaler 256 = 200 ns/tick
 * 45000 = 9000 us
 * 22500 = 4500 us
 * 8437.5 = 1687.5 us
 * 2812.5 = 562.5 us
 */
 
uint8_t tvVolume = 10, tvChannel = 1, power = 0, sigmaTarget = 31, sigmaPrescaler = 255;
/*COME BACK AND REMOVE ME AND JUST USE SINGLE BUFFEr*/
unsigned long irCode = 0x40BE629D;
unsigned long irCodeBuffer = 0x40BE629D;
static uint8_t i = 0;

void tx_Setup() {
  sigma_delta_enable();
  //Sigma Attached to D2 (GPIO 4)
  sigma_delta_attachPin(4);
  sigma_delta_setPrescaler(sigmaPrescaler);
  //Target = 31 to make ~38kHz
  sigma_delta_setTarget(0);
}

//Begin preamble 9000us
void tx_Begin() {
  i = 0;
  irCodeBuffer = irCode;
  sigma_delta_setTarget(sigmaTarget);
  timer1_isr_init();
  timer1_attachInterrupt(preambleISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(45000);
}

//turn off pin for 4500us
void preambleISR(){
  sigma_delta_setTarget(0);
  timer1_attachInterrupt(txBitTime);
  timer1_write(22500);
}

//send 562.5 us
void txBitTime(){
  sigma_delta_setTarget(sigmaTarget);
  timer1_attachInterrupt(txWaitTime);
  timer1_write(2813);
}

//Check opcode
void txWaitTime(){
  sigma_delta_setTarget(0);
  if(i < 32){
    i++;
    timer1_attachInterrupt(txBitTime);
    if(irCodeBuffer & 0x80000000){
       timer1_write(8438);
       irCodeBuffer = irCodeBuffer << 1;
       return;
    }
    else{
      timer1_write(2812);
      irCodeBuffer = irCodeBuffer << 1;
      return;
    }
  }
  //Send stop bit
  Serial.println("stop bit tx");
  sigma_delta_setTarget(sigmaTarget);
  timer1_attachInterrupt(txStopBit);
  timer1_write(2813);
}

void txStopBit(){
  sigma_delta_setTarget(0);
}

