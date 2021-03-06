/* 
 sigma_delta.h - esp8266 sigma-delta source

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
 /*
/******************************************************************************
 * Info Sigma delta module

This module controls the esp8266 internal sigma delta source
Each pin can be connected to the sigma delta source
The target duty and frequency can be modified via the register GPIO_SIGMA_DELTA

THE TARGET FREQUENCY IS DEFINED AS:

FREQ = 80,000,000/prescaler * target /256  HZ,     0<target<128
FREQ = 80,000,000/prescaler * (256-target) /256  HZ,     128<target<256
target: duty ,0-255
prescaler: clk_div,0-255
so the target and prescale will both affect the freq.

Usage :
1. sigma_delta_enable() : activate the sigma delta source with default prescalar (0) & target (0)
2. sigma_delta_attachPin(pin), any pin 0..15, TBC if gpio16 supports sigma-delta source
     This will set the pin to NORMAL output mode (pinMode(pin,OUTPUT))
3. sigma_delta_setPrescaler(uint8_t) : reduce the output frequencies
4. sigma_delta_setTarget(uint8_t) : set the output signal duty cycle, duty cycle = target/256

5. sigma_delta_detachPin(pin), this will revert the pin to NORMAL output mode & GPIO source. 
The sigma delta source remains on until :
6. sigma_delta_disable()

copy this code as example :

#include "sigma_delta.h"

void setup() {

  Serial.begin(115200);
  pinMode(BUILTIN_LED,OUTPUT); // blinkie & sigma-delta mix
  
  uint32 pin = 4; //D2
  sigma_delta_enable();
  //sigma_delta_disable();
  sigma_delta_attachPin(BUILTIN_LED);
  sigma_delta_attachPin(4); // D2
  
  sigma_delta_setPrescaler(255);

  Serial.println();
  Serial.println("Start Sigma Delta Example\n");
  Serial.println("Attached gpio4 to the sigma delta source\n");
  Serial.printf("Current target = %i, prescaler = %i\n",sigma_delta_getTarget(),sigma_delta_getPrescaler());
}

void loop() {
 
  uint8_t target, iRepeat;
  
  Serial.println("Attaching the built in led to the sigma delta source now\n");
  Serial.printf("Current target = %i, prescaler = %i\n",sigma_delta_getTarget(),sigma_delta_getPrescaler());
  sigma_delta_attachPin(BUILTIN_LED);
  
  Serial.println("dimming builtin led...\n");
  for (iRepeat=0;iRepeat<10;iRepeat++)
  {
    for (target=0; target<255;target=target+5)
    {
      sigma_delta_setTarget(target);
      delay(10);
    }
    
    for (target=255; target>0;target=target-5)
    {
      sigma_delta_setTarget(target);
      delay(10);
    }
    
  }
  Serial.println("detaching builtin led & playing a blinkie\n");
  sigma_delta_detachPin(BUILTIN_LED);
  for (iRepeat=0;iRepeat<20;iRepeat++)
  {
    digitalWrite(BUILTIN_LED,!digitalRead(BUILTIN_LED));
    delay(500);
  }
    
}

*******************************************************************************/

#ifndef SIGMA_DELTA_H
#define SIGMA_DELTA_H

#ifdef __cplusplus
extern "C" {
#endif

void sigma_delta_enable(void);
void sigma_delta_disable(void);
void sigma_delta_attachPin(uint8_t pin);
void sigma_delta_detachPin(uint8_t pin);
bool sigma_delta_isPinAttached(uint8_t pin);
uint8_t sigma_delta_getTarget(void);
void sigma_delta_setTarget(uint8_t target);
uint8_t sigma_delta_getPrescaler(void);
void sigma_delta_setPrescaler(uint8_t prescaler);

#ifdef __cplusplus
}
#endif

#endif//SIGMA_DELTA_H
