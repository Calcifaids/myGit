#include <SoftwareSerial.h>
#include <Wire.h>


/*
 * Constant definitions
 */
const uint8_t   irLed = 2;
const uint16_t  bitTime = 562;
static const uint16_t deviceAddress = 0x40BE;

/*
* Operations are as follows:
* 0: Power,   1: Mute,    2: Menu,    3: Source,    4: OK,   
* 5: UP,      6: LEFT,    7: RIGHT,   8: DOWN,      9: Exit
* 10: Vol +,  11: Ch +,   12: Vol -,  13: Ch -,     14: 1
* 15: 2,      16: 3,      17: 4,      18: 5,        19: 6, 
* 20: 7,      21: 8,      22: 9,      23: 0
*/
static const uint16_t opCodes[24] = {0x629D, 0x32CD, 0xA25D, 0xD22D, 0x52AD,
                                    0x12ED, 0x728D, 0x926D, 0xB24D, 0xB04F,
                                    0xF00F, 0x30CF, 0x5AA5, 0x9867, 0x807F,
                                    0x40BF, 0xC03F, 0x20DF, 0xA05F, 0x609F,
                                    0xE01F, 0x10EF, 0x906F, 0x00FF};

/*
 * None-Constant definitions
 */




void setup() {
  /*Join I2C as slave address and add on event function*/
  Wire.begin(8);
  Wire.onReceive(receive_event);

  /*Setup serial for debugging*/
  Serial.begin(115200);
  Serial.println("System Start.");

  /*Setup ir pins*/
  pinMode(irLed, OUTPUT);
  digitalWrite(irLed, LOW);
}

void ir_carrier(unsigned int irMicro){
  /*Scale Micro and loop*/
  for (int i = 0; i < (irMicro / 26); i++){
    /*
     * Transmit Carrier.
     * Numbers adjusted to make 13 micro per half of the carrier
     * Digital write last ~4 micro so set delay is set to 9
     * 
     * ?Consider switching to a soft delay?
     */
     digitalWrite(irLed, HIGH);
     delayMicroseconds(9);
     digitalWrite(irLed, LOW);
     delayMicroseconds(9);
  }
}

/*
 * Function currently set to send NEC protocol
 * High = 3 * bit time
 * Low = 1 * bit time
 */
void ir_send_code(unsigned long code){
  /*Send preamble to TV*/
  ir_carrier(9000);
  delayMicroseconds(4500);

  /*Send the 4 byte code*/
  for (int i = 0; i < 32; i++){
    ir_carrier(bitTime);

    /*Check if something in MSB to tx*/
    if (code & 0x80000000){
      delayMicroseconds(3 * bitTime);
    }
    else{
      delayMicroseconds(bitTime);
    }
    /*Shift next bit in to place*/
    code = code << 1;
  }
  /*Send stop bit*/
  ir_carrier(bitTime);
}

void loop() {
  /*Loop forever waiting for I2C tx*/
  while(1);

}

void receive_event(int howMany){
  /*create buffer*/
  char c[3] = {};
  int i;

  /*Itterate through values and place on to char array*/
  while (0 < Wire.available()){
    c[i] = Wire.read();
    /*Kept for debugging purposes*/
    Serial.print(c[i]);
    i++;
  }

  Serial.println();

  /*Convert char array to int*/
  int result = atoi(c);
  
  /*Clear previous opcode and mask device addres and new opcode*/
  unsigned long  irCode = 0;
  irCode = irCode | deviceAddress;
  irCode = irCode << 16;
  irCode = irCode | opCodes[result];

  /*Print full code for debugging*/
  Serial.println(irCode, HEX);

  /*Start tx*/
  ir_send_code(irCode);
  Serial.println();
}





