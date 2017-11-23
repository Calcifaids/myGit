#include <SPI.h>
#include <Ethernet2.h>
#include <PubSubClient.h>

byte mac[] = {0x90, 0xA2, 0xDA, 0x11, 0x3B, 0xC6};

IPAddress ip(192, 168, 0, 11);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

EthernetClient ethernet_client;
PubSubClient client(ethernet_client);

int currentButtonState = LOW;
int lastButtonState = LOW;
int toggleButton;
unsigned int threshold = 30;
unsigned int maxLevel = 0;
unsigned int minLevel = 1023;
float ldrValue; 

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long ts = millis();

int ledState = LOW;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting MQTT client on arduino ...");
  
  client.setServer("io.adafruit.com", 1883);
  client.setCallback(callback);
  
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  
  if (Ethernet.begin(mac) == 0){
    Serial.println("Failed to configure ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }
   
   delay(1500);
   
   Serial.print("MQTT client is at: ");
   Serial.println(Ethernet.localIP());
   
 
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()){
    reconnect();
  }
  
  updateLightState();
  digitalWrite(3, ledState);

  
  if (millis() > ts + 1500){
    ts = millis();
    sendLight(&maxLevel, &minLevel);
  }
  
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int messLength){
  String t = String(topic);
  
  char data[messLength+1];
  for (int i = 0; i < messLength; i++){
    data[i] = payload[i]; 
  }
  data[messLength] = '\0';
  
  Serial.print("message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(data);
  
  if (t.indexOf("light-switch") > 0){
    
    if (strcmp(data, "ON") == 0){
      if (toggleButton == LOW){
        ledState = HIGH; 
      }
      else{
        //Reset lightswitch while threshold button on
        client.publish("calcifaids/f/light-switch", "OFF");
      }
    }
    else if (strcmp(data, "OFF") == 0){
      ledState = LOW; 
    }
  }

  if (t.indexOf("toggle-threshold") > 0){

    if (strcmp(data, "ON") == 0){
     toggleButton = HIGH; 
    }
    else if (strcmp(data, "OFF") == 0){
      toggleButton = LOW; 
      ledState = LOW;
    }
  }

  if (t.indexOf("calibrate-ldr") > 0){
    if (strcmp(data, "1") == 0){
      calibrateLdr(&maxLevel, &minLevel);
    }
  }

  if (t.indexOf("light-threshold") > 0){
    threshold = atoi(data);
  }
}

void sendLightState(int led){
  if (client.connected()){
    if (led == HIGH){
      client.publish("calcifaids/f/light-switch", "ON"); 
    }
    else{
      client.publish("calcifaids/f/light-switch", "OFF");
    }
  } 
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT Connection...");

    if (client.connect("Callums_Arduino", "calcifaids", "bb027dc7a55644929190949f85d6ac1b")) {
      Serial.println("... connected");

      client.publish("calcifaids/f/status-messages", "we are alive!");

      sendLightState(ledState);

      client.subscribe("calcifaids/f/message-log");
      client.subscribe("calcifaids/f/light-switch");
      client.subscribe("calcifaids/f/calibrate-ldr");
      client.subscribe("calcifaids/f/light-threshold");
      client.subscribe("calcifaids/f/toggle-threshold");
      calibrateLdr(&maxLevel, &minLevel);
      //Threshold reset on initialisation of programme
      String threshString = String(threshold, DEC);
      char stringBuffer[3];
      threshString.toCharArray(stringBuffer, 3);
      client.publish("calcifaids/f/light-threshold",stringBuffer);
    }
    else {
      Serial.print("Failed, rc = ");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");

      delay(5000);
    }
  }
}

void updateLightState(){
  if (toggleButton == HIGH){
    int light = analogRead(A0);
    if (light <= threshold){
      ledState = HIGH;
    }
    else{
      ledState = LOW;
    }
  }
  else{
    int reading = digitalRead(2);
    
    if(reading != lastButtonState){
      lastDebounceTime = millis(); 
    }
  
    if((millis() - lastDebounceTime) > debounceDelay){
      if (reading != currentButtonState){
        currentButtonState = reading;
        
        if (currentButtonState == HIGH){
           ledState = !ledState;
           sendLightState(ledState); 
        }
      }
    }
    
    lastButtonState = reading;
  }
}

void calibrateLdr(uint16_t *maxL, uint16_t *minL){
  unsigned long timestamp = millis();
  uint8_t i = 1;
  *maxL = 0;
  *minL = 1023;
  Serial.println("Begin calibration now:");
  while (millis() < (timestamp + 5000)){
    if ((i == 1) && (millis() < timestamp + 1000)){
      i++;
      Serial.println("5");
    }
    else if ((i == 2) && (millis() > timestamp + 1000) && (millis() < timestamp + 2000)){
      i++;
      Serial.println("4");
    }
    else if ((i == 3) && (millis() > timestamp + 2000) && (millis() < timestamp + 3000)){
      i++;
      Serial.println("3");
    }
    else if ((i == 4) && (millis() > timestamp + 3000) && (millis() < timestamp + 4000)){
      i++;
      Serial.println("2");
    }
    else if ((i == 5) && (millis() > timestamp + 4000) && (millis() < timestamp + 5000)){
      i++;
      Serial.println("1");
    }
    int light = analogRead(A0);
    if (light > *maxL){
      *maxL= light;
    }
    if (light < *minL){
      *minL = light;
    }
  }
  client.publish("calcifaids/f/message-log","LDR recalibrated");
}

void sendLight(uint16_t *maxL, uint16_t *minL){
  if (client.connected()){
    int light = analogRead(A0);
    light = map(light, *minL, *maxL, 0, 100);
    if(light >= *maxL){
      light = 100;
    }

    String lightString = String(light, DEC);
    char stringBuffer[3];
    lightString.toCharArray(stringBuffer, 3);
    client.publish("calcifaids/f/light-levels",stringBuffer);
    
  }
}

