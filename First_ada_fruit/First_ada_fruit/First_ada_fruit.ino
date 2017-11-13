#include <PubSubClient.h>
#include <SPI.h>
#include <Ethernet2.h>



byte mac[] = {0x90, 0xA2, 0xDA, 0x11, 0x3B, 0xC6};

//Setup for Home Network
IPAddress ip(192, 168, 1, 250);
IPAddress gateway(192, 169, 1, 254);
IPAddress subnet(255, 255, 255, 0);

//MQTT declarations
//Get ethernet client & pass to MQTT client
EthernetClient ethernet_client;
PubSubClient client(ethernet_client);

//Programme wide variables
long timestamp = millis();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting MQTT client on Arudino...");

  client.setServer("io.adafruit.com", 1883);
  client.setCallback(callback);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  //Delay to ensure all set up ok
  delay(1500);

  Serial.print("MQTT client is at: ");
  Serial.println(Ethernet.localIP());

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  //Publish rng every 5s
  if (millis() > timestamp + 5000) {
    timestamp = millis();
    send_rng();
    send_light();
  } 
  client.loop();
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT Connection...");

    if (client.connect("Callums_Arduino", "calcifaids", "bb027dc7a55644929190949f85d6ac1b")) {
      Serial.println("... connected");

      client.publish("calcifaids/f/status-messages", "we are alive!");

      //client.subscribe("calcifaids/f/#");
      client.subscribe("calcifaids/f/test-slider");
      client.subscribe("calcifaids/f/random-numbers");
      client.subscribe("calcifaids/f/light-levels");
    }
    else {
      Serial.print("Failed, rc = ");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");

      delay(5000);
    }
  }

}

void send_rng(){
  if (client.connected()){
    int rando = random(100);
    String randoString = String(rando, DEC);
    char mystring[5];
    randoString.toCharArray(mystring, 5);

    client.publish("calcifaids/f/random-numbers", mystring);
  }
}

void send_light(){
  if (client.connected()){
    int light = analogRead(A0);
    
    char light_level[5];
    sprintf(light_level, "%i", light);
    client.publish("calcifaids/f/light-levels",light_level);
  }
}
