#include <LiquidCrystal.h>


/*
* analog_blink_rate.ino
*
* simple program to adjust the blink rate of an LED based
* upon the value of a potentiometer.
*
*/
// set pin numbers
const int rs = 13, en = 12, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int warningLed = 11;
const int temperatureLed = 10;
const int warningPotPin = A0;
const int heatingPotPin = A1;
const int tempPin = A2;
const int millisDelay = 5000;
// initialise the state of the led
int writeState = 0;
int warningState = LOW;
int heatingState = LOW;
int temperature, warningLimit, heatingLimit;
int intermediary;
float tempMiliVoltage;
unsigned long timeStamp = 0;
unsigned long previousTime = 0;

// set up the initial states of the program
void setup(){
  // set up the serial connection to enable debugging
  Serial.begin(9600);
  // set the pin i/o
  pinMode(warningLed, OUTPUT);
  pinMode(temperatureLed, OUTPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}
// main program
void loop(){
  /*Sample Pots for limits*/
  warningLimit = setWarning();
  heatingLimit = setHeating();

  // get the value of the pot
  int tempVal = analogRead(tempPin);
  tempMiliVoltage = (tempVal * 5.0) / 1024;
  temperature = (tempMiliVoltage - 0.5) * 100;
  // debugging

 
  /*Check if 11, 10, 01, 00 for LEDs*/
  if((temperature >= warningLimit) && (temperature <= heatingLimit)){
    digitalWrite(warningLed, HIGH);
    digitalWrite(temperatureLed, HIGH);
  }
  else if((temperature >= warningLimit) && (temperature > heatingLimit)){
    digitalWrite(warningLed, HIGH);
    digitalWrite(temperatureLed, LOW);
  }
  else if ((temperature < warningLimit) && (temperature <= heatingLimit)){
    digitalWrite(warningLed, LOW);
    digitalWrite(temperatureLed, HIGH);
  }
  else{
    digitalWrite(warningLed, LOW);
    digitalWrite(temperatureLed, LOW);
  }


  if ((millis() - previousTime) >= millisDelay){
    previousTime = millis();
    Serial.print("Temp = ");
    Serial.println(temperature, DEC);
    Serial.print("Warning = ");
    Serial.println(warningLimit);
    Serial.print("Heating  = ");
    Serial.println(heatingLimit);
    if (writeState == 0){
    writeState = 1;
      lcd.clear();
      lcd.setCursor(0,0);
      temperature = (int)temperature;
      lcd.print("Temp = ");
      lcd.println(temperature);
      lcd.setCursor(0,1);
      lcd.print("Warning = ");
      lcd.println(warningLimit);
    }
    else{
      writeState = 0;
      lcd.clear();
      lcd.setCursor(0,0);
      temperature = (int)temperature;
      lcd.print("Temp = ");
      lcd.println(temperature);
      lcd.setCursor(0,1);
      lcd.print("Heating = ");
      lcd.println(heatingLimit);
    }
  }
}

int setWarning(){
  //Read value on Warning pot
  int warningValue = analogRead(warningPotPin);
  //convert to value between 15 -> 40
  warningValue = (warningValue * 25 ) / 1024 + 15;
  return warningValue;
}

int setHeating(){
  //Read value on heating Pot
  int heatingValue = analogRead(heatingPotPin);
  //convert to value between 0 > 25
  heatingValue = (heatingValue * 25) / 1024;
  return heatingValue;
}

