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
 
uint8_t tvVolume = 10, tvChannel = 1, power = 0, sigmaTarget = 31, sigmaPrescaler = 255, txMutex = 0;

//Change from const in later itterations to support multi address
static const uint16_t deviceAddress = 0x40BE;
/*
* Operations are as follows:
* 1: Power,   2: Mute,    3: Menu,    4: Source,    5: OK,   
* 6: UP,      7: LEFT,    8: RIGHT,   9: DOWN,      10: Exit
* 11: Vol +,  12: Ch +,   13: Vol -,  14: Ch -,     15: 1
* 16: 2,      17: 3,      18: 4,      19: 5,        20: 6, 
* 21: 7,      22: 8,      23: 9,      24: 0
*/
static const uint16_t opCodes[25] = {0, 0x629D, 0x32CD, 0xA25D, 0xD22D, 0x52AD,
                                    0x12ED, 0x728D, 0x926D, 0xB24D, 0xB04F,
                                    0xF00F, 0x30CF, 0x5AA5, 0x9867, 0x807F,
                                    0x40BF, 0xC03F, 0x20DF, 0xA05F, 0x609F,
                                    0xE01F, 0x10EF, 0x906F, 0x00FF};

/*COME BACK AND REMOVE ME AND JUST USE SINGLE BUFFEr*/
unsigned long irCodeBuffer;
static uint8_t i = 0;

static int8_t operationBuffer[8] = {0};


void txSetup() {
  sigma_delta_enable();
  //Sigma Attached to D2 (GPIO 4)
  sigma_delta_attachPin(4);
  sigma_delta_setPrescaler(sigmaPrescaler);
  //Target = 31 to make ~38kHz
  sigma_delta_setTarget(0);
}


bool checkTxMutex(){
  //if in use return false
  if (txMutex == 0 && operationBuffer[0] != 0){
    return true;
  }
  else{
    return false;
  }
}

void addToBuffer(uint8_t operation){
  for (int i = 0; i < 8; i++){
    if(operationBuffer[i] == 0){
      operationBuffer[i] = operation;
      return;
    }
  }
  Serial.println("If you have reached here, buffer is full and operation has been dropped");
}

void shiftOffBuffer(){
  for (int i = 0; i < 8; i++){
    if (operationBuffer[i] == 0){
      //Finished itteration
      Serial.println("finished the shift");
      return;
    }
    operationBuffer[i] = operationBuffer[i+1];
    Serial.println("In the operation shift");
  }
}

//Create full code, create preamble timer and start
void txBegin() {
  txMutex = 1;
  i = 0;
  irCodeBuffer = irCodeBuffer | deviceAddress;
  irCodeBuffer = irCodeBuffer << 16;
  irCodeBuffer = irCodeBuffer | opCodes[operationBuffer[0]];
  shiftOffBuffer();
  Serial.print("Transmitting value ");
  Serial.println(irCodeBuffer, HEX);

  //Setup ISR & TX start preamble
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

//Itterate through current bit of opCode each cycle delay respectively
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
  sigma_delta_setTarget(sigmaTarget);
  timer1_attachInterrupt(txStopBit);
  timer1_write(2813);
}

void txStopBit(){
  sigma_delta_setTarget(0);
  unsigned long timestamp = millis();
  Serial.println(timestamp);
  txMutex = 0;
}

