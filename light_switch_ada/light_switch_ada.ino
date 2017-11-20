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

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

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
     ledState = HIGH; 
    }
    else if (strcmp(data, "OFF") == 0){
      ledState = LOW; 
    }
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

      //client.subscribe("calcifaids/f/#");
      client.subscribe("calcifaids/f/message-log");
      client.subscribe("calcifaids/f/light-switch");
      
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

