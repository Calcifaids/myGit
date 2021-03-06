#include <Ethernet2.h>
#include <SPI.h>

#include <aREST.h>

#include <avr/wdt.h>


EthernetServer server = EthernetServer(80);

IPAddress ip(192, 168, 0, 11);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

byte mac[] = {0x90, 0xA2, 0xDA, 0x11, 0x3B, 0xC6};

aREST rest = aREST();

int lightLevel, potLevel, tempLevel;
int sensorMax = 0;
int sensorMin = 1023;
uint8_t led = 3;
int pwm;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting rest web service on arduino ...");
  
  //Set pin modes
  //A0 ldr, A1 pot, A2 temp
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  
  //Calibrate LDR
  Serial.println("Please calibrate LDR now.");
  while (millis() < 5000){
    int sensorVal = analogRead(A0);
    if (sensorVal > sensorMax){
      sensorMax = sensorVal;
    }
    if (sensorVal < sensorMin){
      sensorMin = sensorVal;
    }
  }
  Serial.println("Calibration Finished.");
  
  //Init adc tied components and expose to restAPI
  lightLevel = 0;
  potLevel = 0;
  tempLevel = 0;
  rest.variable("light_level", &lightLevel);
  rest.variable("pot_level", &potLevel);
  rest.variable("temp_level", &tempLevel);
  rest.function("led", led_control);
  rest.function("switch_led", switch_led);
  rest.function("random_number", random_number);
  
  rest.set_id("001");
  rest.set_name("callums_arduino");
  
  if (Ethernet.begin(mac) == 0){
    Serial.println("failed to configure ethernet using DHCP");
    Ethernet.begin(mac, ip); 
  }
  server.begin();
  Serial.print("web service is at: ");
  Serial.println(Ethernet.localIP());
  
  wdt_enable(WDTO_4S);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //Get light
  lightLevel = analogRead(A0);
  lightLevel = map(lightLevel, sensorMin, sensorMax, 0, 100);
  if (lightLevel >= sensorMax){
    lightLevel = 100; 
  }
  
  //Get pot
  potLevel = analogRead(A1);
  
  //Get temp
  tempLevel = analogRead(A2);
  float tempConv = (tempLevel * 5.0) / 1024.0;
  tempConv = (tempConv - 0.5) * 100;
  tempLevel = tempConv;
  
  EthernetClient client = server.available();
  rest.handle(client);
  wdt_reset();
}

int led_control(String command){
  Serial.print("Command is ");
  Serial.println(command);
 
  pwm = command.toInt();
  pwm = constrain(pwm, 0, 255);
 
  analogWrite(led, pwm);
  return 1; 
}

int switch_led(String command){
  Serial.print("Command is ");
  Serial.println("switch led");
  
  //all off
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  
  led ++;
  switch (led){
    case 4:
      analogWrite(led, pwm);
    break;
    case 5:
      analogWrite(led, pwm);
    break;
    case 6:
      analogWrite(led, pwm);
    break;
    case 7:
      led = 3;
      analogWrite(led, pwm);
    break;
  }
  return 1;
}

int random_number(String command){
  uint8_t randomVar = random(1,6); 
  
  return randomVar;
}
