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

int lightLevel;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting rest web service on arduino ...");
  
  //Set pin modes
  pinMode(3, OUTPUT);
  pinMode(A0, INPUT);
  
  //Init light level and expose to restAPI
  lightLevel = 0;
  rest.variable("light_level", &lightLevel);
    
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
  lightLevel = analogRead(A0);
  
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
