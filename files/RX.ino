/* 
 * RemoteSensorRX [Reception Unit]
 * Hardware: Arduino Uno + Ethernet shield Enc28J60
 * @author: Vanderlei Mendes
 * Project to solve a problem of my friend Gustavo (Só Portões)
 * 
 * This is the Receiver (server) part wich is intended to 
 * be used with another Arduino Uno + ENC28J60 as a client/transmitter
 * 
 * When connected to an alarm system, it can receive the same status open/close from the remote
 * Unit over Local Area Network (LAN) from an alarm detector 
 * It's even possible to receive over WAN if you make the forwarding rule in your router
 * 
 * Note: No Security authentication layer is provided in this project. 
 * Use as it is only for test purposes
 */

#include <EtherCard.h>

#define led A4
#define trigger A5


static byte mymac[] = {0x74, 0x69, 0x68, 0x2D, 0x37, 0x31};
static byte myip[] = {192, 168, 25, 16};
byte Ethernet::buffer[700];


void setup()
{

  Serial.begin(57600);
  Serial.println(F("\n[Remote sensor Receptor ( RX Unit )]"));

  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println("Failed to access Ethernet controller");
  else
    Serial.println("Ethernet controller initialized");

  if (!ether.staticSetup(myip))
    Serial.println("Failed to set IP address");

  Serial.println();

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);  

  pinMode(trigger, OUTPUT); 
  digitalWrite(trigger, LOW);
}

void loop()
{
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos)
  {    
    if (strstr((char *)Ethernet::buffer + pos, "GET /?txrelay=opened") != 0)
    {
      Serial.println("\n---------- NEW REQUEST: CONTACT IS OPENED ----------");      
      Serial.println("Motion reported by the Transmitter Unit. Alarm trigger is on!");
      digitalWrite(led, HIGH);
      digitalWrite(trigger, HIGH);
      BufferFiller bfill = ether.tcpOffset();
      bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\n"
                      "Content-Type: application/json\r\nPragma: no-cache\r\n\r\n"
                      "{\"rxrelay\":\"opened\"}"));
      ether.httpServerReply(bfill.position());     
    }
    
    if (strstr((char *)Ethernet::buffer + pos, "GET /?txrelay=closed") != 0)
    {
      Serial.println("\n---------- NEW REQUEST:CONTACT IS CLOSED----------");
      Serial.println("Restore command reported by the Transmitter Unit. Alarm trigger is off!");
      digitalWrite(led, LOW);
      digitalWrite(trigger, LOW);      
      BufferFiller bfill = ether.tcpOffset();
      bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\n"
                      "Content-Type: application/json\r\nPragma: no-cache\r\n\r\n"
                      "{\"rxrelay\":\"closed\"}"));
      ether.httpServerReply(bfill.position());     
    }    
  }  
}
