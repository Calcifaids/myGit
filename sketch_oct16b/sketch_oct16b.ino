#include <Ethernet2.h>
#include <SPI.h>

  EthernetServer server = EthernetServer(80);
  
  IPAddress ip(192, 168, 0, 6);
  IPAddress gateway(192, 168, 0, 254);
  IPAddress subnet(255, 255, 255, 0);
  
  //Change me on startup
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0xF5, 0xDE};

  const int potPin = A0;
  const int ldrPin = A1;
  boolean payAttention = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  if (Ethernet.begin(mac) == 0){
    Serial.println("failed to configure ethernet using DHCP");
    Ethernet.begin(mac, ip); 
  }
 server.begin();
 Serial.println(Ethernet.localIP());
}

void loop(){
  checkForConnections();
}


void checkForConnections() {
  EthernetClient client = server.available();
  
  boolean headerSent = false;
  
  while(client){
    if(!headerSent){
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      headerSent = true;
    }

     if (client.available() > 0){
       char c = client.read();
       
       if (c == '?'){
        payAttention = true; 
       }
       else if (c == ' '){
        payAttention = false; 
       }
       
       if (payAttention){
         switch (c){
           case '2':
             triggerPin(2, client);
             break;
           case '3':
             triggerPin(3, client);
             break;
           case '4':
             triggerPin(4, client);
             break;
           case '5':
             triggerPin(5, client);
             break;
           case 'p':
             readPot(potPin, client);
             break;
           case 'l':
             readPot(ldrPin, client);
             break; 
           case 't':
        
             break;   
           case 'x':
             clearPins(client);
             break;
         } 
       }
     }
     else{
          client.stop(); 
     }
       
       
       /*OLD WEBPAGE CODE*/
//        Serial.print((char)client.read());
//        client.println("<html>");
//        client.println("<head><style type = 'text/css'>body{background-color: brown;}p{font: 20px arial, sans-serif; color: green;}</style>My Web Page</head>");
//        client.println("<h1>Hello World</h1></hr><cr><p>This is an <strong>example</strong> <a href='http://www.brain.engineering'>page</a></p>");
//        client.println("</body></html>");
//        client.stop();
   }
}

void readPot(int pot, EthernetClient client){
   client.print("Value on Pot pin <b>");
   client.println(pot);
   client.println("</b> = <b>");
   float analogReadVal = analogRead(pot);
   client.println(analogReadVal);
   client.println("</b></br>");
}

void readLDR

void triggerPin(int pinNumber, EthernetClient client){
  client.print("Turning on pin <b>");
  client.println(pinNumber);
  client.println("</b><br>");
 
 digitalWrite(pinNumber, HIGH); 
}

void clearPins(EthernetClient client){
  client.println("Clearing all pins!<br>");
 
 //Turn off pins
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);  
}
