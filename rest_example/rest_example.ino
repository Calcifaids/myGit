#include <Ethernet2.h>
#include <SPI.h>

#include <aREST.h>

#include <avr/wdt.h>


EthernetServer server = EthernetServer(80);

IPAddress ip(192, 168, 0, 11);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

byte mac[] = {0x90, 0xA2, 0xDA, 0x11, 0x44, 0xB6};

aREST rest = aREST();

int lightLevel, potLevel, tempLevel;
int sensorMax = 0;
int sensorMin = 1023;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting rest web service on arduino ...");
  
  //Set pin modes
  pinMode(3, OUTPUT);
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
 
  int pwm = command.toInt();
  pwm = constrain(pwm, 0, 255);
 
  analogWrite(3, pwm);

  return 1; 
}
