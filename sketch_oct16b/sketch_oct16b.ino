#include <Ethernet2.h>
#include <SPI.h>

  EthernetServer server = EthernetServer(80);
  
  IPAddress ip(192, 168, 0, 6);
  IPAddress gateway(192, 168, 0, 254);
  IPAddress subnet(255, 255, 255, 0);
  
  byte mac[] = {0x90, 0xA2, 0xDA, 0x11, 0x44, 0xA8};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (Ethernet.begin(mac) == 0){
    Serial.println("failed to configure ethernet using DHCP");
    Ethernet.begin(mac, ip); 
  }
 Serial.print("Hello World!");
 server.begin();
 Serial.println(Ethernet.localIP());
 
}

void loop() {
  // put your main code here, to run repeatedly:
  EthernetClient client = server.available();
  
  while(client){
     if (client.available() > 0){
        Serial.print((char)client.read());
     }
     else{
      client.println("HTTP/1.1 200 OK"); 
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html>");
      client.println("<head><style type = 'text/css'>body{background-color: brown;}p{font: 20px arial, sans-serif; color: green;}</style>My Web Page</head>");
      client.println("<h1>Hello World</h1></hr><cr><p>This is an <strong>example</strong> <a href='http://www.brain.engineering'>page</a></p>");
      client.println("</body></html>");
      client.stop();
     }
  }
}
