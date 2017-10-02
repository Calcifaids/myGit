/*
* analog_blink_rate.ino
*
* simple program to adjust the blink rate of an LED based
* upon the value of a potentiometer.
*
*/
// set pin numbers
const int warningLed = 11;
const int temperatureLed = 10;
const int warningPotPin = A0;
const int heatingPotPin = A1;
const int tempPin = A2;
const int millisDelay = 5000;
// initialise the state of the led
int warningState = LOW;
int heatingState = LOW;
int temperature, warningLimit, heatingLimit;
float tempMiliVoltage;
unsigned long previousTime = 0;

// set up the initial states of the program
void setup(){
  // set up the serial connection to enable debugging
  Serial.begin(9600);
  // set the pin i/o
  pinMode(warningLed, OUTPUT);
  pinMode(temperatureLed, OUTPUT);
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
    Serial.print("Heating Limit = ");
    Serial.println(heatingLimit);
    
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
