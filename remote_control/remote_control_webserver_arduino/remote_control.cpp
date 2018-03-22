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
 
uint8_t tvVolume = 10, resetVolume = 10, tvVolumeThreshold = 80, power = 0, sigmaTarget = 31, sigmaPrescaler = 255, txMutex = 0, macroFlag = 0, delaySeconds = 0;
int32_t delayTime = 5000000;

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
static uint8_t iterator = 0;

static int8_t operationBuffer[8] = {0};


void txSetup() {
  sigma_delta_enable();
  //Sigma Attached to D2 (GPIO 4)
  sigma_delta_attachPin(4);
  sigma_delta_setPrescaler(sigmaPrescaler);
  //Target = 31 to make ~38kHz
  sigma_delta_setTarget(0);
}

bool updateVolumeThreshold(uint8_t newThresh){
  //Update threshold
  tvVolumeThreshold = newThresh;
  //Check if volume needs adjusting with new threshold


  return true;
}

//Returns whether it's suitable to add to buffer.
bool checkToAddBuffer(){
  if (macroFlag == 0){
    return true;
  }
  else{
    return false;
  }
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
  iterator = 0;
  //Check for power command
  if(operationBuffer[0] == 1){
    //TV turning on
    if(power == 0){
      //macroFlag value dictates action
      macroFlag = 1;
      power = 1;
      delaySeconds = 10;
      irCodeBuffer = irCodeBuffer | deviceAddress;
      irCodeBuffer = irCodeBuffer << 16;
      irCodeBuffer = irCodeBuffer | opCodes[1];
      shiftOffBuffer();
      Serial.print("Transmitting value ");
      Serial.println(irCodeBuffer, HEX);
    
      startPreamble();
    }
    //TV turning off
    else{
      Serial.println("in TV turning off section");
      /*FIX VOLUME ADJUST WHEN OFF*/
      //Setup blocking mutex
      txMutex = 2;
      macroFlag = 2;
      //Clear operation buffer to prevent any unwanted 
      for (int i = 0; i < 8; i++){
        operationBuffer[i] = 0;
      }
      //Determine size 
      int8_t macroVolumeChange = tvVolume - resetVolume;
      //Check and send first volume change + send to hold
      if (macroVolumeChange < 0){
        //volume up
        volumeUp();
      }
      else if (macroVolumeChange > 0){
        //Volume down 
        volumeDown();
      }
      else{
        //set flag to 3 & start from channel
        macroFlag = 3;
        //Send Ch 1
        irCodeBuffer = irCodeBuffer | deviceAddress;
        irCodeBuffer = irCodeBuffer << 16;
        irCodeBuffer = irCodeBuffer | opCodes[15];
        
        Serial.print("Transmitting value ");
        Serial.println(irCodeBuffer, HEX);
      
        startPreamble();
      }
    }
  }
  //Do normal
  else{
    /*MAKE MORE ROBIUST*/
    Serial.println("okToSend set to 1");
    uint8_t okToSend = 1;
    if (power != 0){
      switch(operationBuffer[0]){
        case 11:
          //Volume up if within threshold
          if (tvVolume < tvVolumeThreshold){
            tvVolume ++;
            Serial.print(F("New volume = "));
            Serial.println(tvVolume);
          }
          else{
            Serial.println("okToSend set to 0");
            okToSend = 0;
          }
          break;
        case 13:
          //Volume down if within threshold
          if (tvVolume > 0){
            tvVolume --;
            Serial.print(F("New volume = "));
            Serial.println(tvVolume);
          }
          else{
            Serial.println("okToSend set to 0");
            okToSend = 0;
          }
          break;
      }
    }
    irCodeBuffer = irCodeBuffer | deviceAddress;
    irCodeBuffer = irCodeBuffer << 16;
    irCodeBuffer = irCodeBuffer | opCodes[operationBuffer[0]];
    shiftOffBuffer();
    
    if (okToSend == 1){
      Serial.print(F("Transmitting value "));
      Serial.println(irCodeBuffer, HEX);
      startPreamble();
    }
    else{
      //Not sending volume change so release mutex
      txMutex = 0;
    }
  }
}


void startPreamble(){
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
  if(iterator < 32){
    iterator++;
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
  iterator = 0;
  //Send stop bit
  sigma_delta_setTarget(sigmaTarget);
  timer1_attachInterrupt(txStopBit);
  timer1_write(2813);
}

void txStopBit(){
  sigma_delta_setTarget(0);
  unsigned long timestamp = millis();
  Serial.println(timestamp);
  //Check if delay needed for macro power ON/OFF
  if(macroFlag == 0){
    txMutex = 0;
    Serial.println(F("Freeing mutex"));
  }
  else if(macroFlag == 1){
    Serial.println(F("Delaying to make sure the TV is clear for operation"));
    timer1_attachInterrupt(powerOnDelay);
    timer1_write(delayTime);
  }
  else {
    timer1_attachInterrupt(delayThenHandle);
    timer1_write(1000000);
  }
  
  
}

void volumeUp(){
  tvVolume ++;
  irCodeBuffer = irCodeBuffer | deviceAddress;
  irCodeBuffer = irCodeBuffer << 16;
  irCodeBuffer = irCodeBuffer | opCodes[11];
  Serial.print("Transmitting value ");
  Serial.println(irCodeBuffer, HEX);

  startPreamble();
}

void volumeDown(){
  tvVolume --;
  irCodeBuffer = irCodeBuffer | deviceAddress;
  irCodeBuffer = irCodeBuffer << 16;
  irCodeBuffer = irCodeBuffer | opCodes[13];
  Serial.print("Transmitting value ");
  Serial.println(irCodeBuffer, HEX);

  startPreamble();
}

void powerOnDelay(){
  static int i = 0;
  //Recursively set timer until correct delay has passed
  if (i < delaySeconds){
    i++;
    timer1_attachInterrupt(powerOnDelay);
    timer1_write(delayTime);
  }
  else{
    i = 0;
    Serial.println("finished delay");
    txMutex = 0;
    macroFlag = 0;
  }
}

void delayThenHandle(){
  
  if(macroFlag == 2){
    //Continue changing volume
    if (tvVolume < resetVolume){
      //Send volume up command
      volumeUp();
    }
    else if(tvVolume > resetVolume){
      //Send volume down command
      volumeDown();
    }
    else{
      macroFlag = 3;
      //Send Ch 1
      irCodeBuffer = irCodeBuffer | deviceAddress;
      irCodeBuffer = irCodeBuffer << 16;
      irCodeBuffer = irCodeBuffer | opCodes[15];
      Serial.print("Transmitting value ");
      Serial.println(irCodeBuffer, HEX);
    
      startPreamble();
    }
  }
  else if(macroFlag == 3){
    macroFlag = 4;
    //Send OK
    irCodeBuffer = 0;
    irCodeBuffer = irCodeBuffer | deviceAddress;
    irCodeBuffer = irCodeBuffer << 16;
    irCodeBuffer = irCodeBuffer | opCodes[5];
    Serial.print("Transmitting value ");
    Serial.println(irCodeBuffer, HEX);
  
    startPreamble();
  }
  else{
    macroFlag = 1;
    power = 0;
    delaySeconds = 3;
    //Send power
    irCodeBuffer = 0;
    irCodeBuffer = irCodeBuffer | deviceAddress;
    irCodeBuffer = irCodeBuffer << 16;
    irCodeBuffer = irCodeBuffer | opCodes[1];
    Serial.print("Transmitting value ");
    Serial.println(irCodeBuffer, HEX);
  
    startPreamble();
  }
}

