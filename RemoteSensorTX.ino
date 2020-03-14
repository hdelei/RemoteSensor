/* 
 * RemoteSensorTX [Transmission Unit]
 * Hardware: Arduino Uno + Ethernet shield Enc28J60
 * @author: Vanderlei Mendes
 * Project to solve a problem of my friend Gustavo (Só Portões)
 * 
 * This is the Client part wich is intended to 
 * be used with another Arduino Uno + ENC28J60 as a server/receptor
 * 
 * When connected to a N.C. alarm sensor, it can send the same status open/close to the remote
 * Unit over Local Area Network (LAN) to an alarm system 
 * It's even possible send over WAN if you make the forwarding rule in your router
 * 
 * Note: No Security authentication layer is provided in this project. 
 * Use as it is only for test purposes
 */

#include <EtherCard.h>

#define led A4
#define trigger A3

boolean motionCondition = true;
boolean restoreCondition = false;

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x68,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];

const char website[] PROGMEM = "ardudoor.free.beeceptor.com"; //MOCKING SERVER FOR TESTS

// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  String response;
  
  response = String((char*) Ethernet::buffer + off);
  Serial.println(response);

  checkRemote(response);  
  //Serial.println("...");
}

static void checkRemote(String resp){  
  if(resp.substring(9,12) == "200"){
    Serial.println("## COMMAND SUCCESSFUL PERFOMED ON REMOTE DEVICE ##");
  }
  else{
    Serial.println("## BAD REQUEST OR NO RESPONSE FROM REMOTE DEVICE  ##");
    //blink a red led in case of a bad request
  }
}

void setup () {
  
  Serial.begin(57600);
  Serial.println(F("\n[Remote sensor Transmitter ( TX Unit )]"));

  // Change 'SS' to your Slave Select pin, if you arn't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("Gateway:  ", ether.gwip);
  ether.printIp("DNS server: ", ether.dnsip);


  //REMOTE IP ADDRESS
  byte hisip[] = { 192,168,25,49 };
  ether.copyIp(ether.hisip, hisip);
  //-------------------------------

//--------------- BLOCK USED ONLY WHEN YOU NEED TO REQUEST BY DNS --------------
// #if 1
//   // use DNS to resolve the website's IP address
//   if (!ether.dnsLookup(website))
//     Serial.println("DNS failed");    
// #elif 2
//   // if website is a string containing an IP address instead of a domain name,
//   // then use it directly. Note: the string can not be in PROGMEM.
//   char websiteIP[] = "192.168.1.1";
//   ether.parseIp(ether.hisip, websiteIP);
// #else
//   // or provide a numeric IP address instead of a string
//   byte hisip[] = { 192,168,1,1 };
//   ether.copyIp(ether.hisip, hisip);
// #endif
//----------------------------------------------------------

  ether.printIp("Server IP: ", ether.hisip);

  pinMode(trigger, INPUT_PULLUP);
  pinMode(led, OUTPUT);   
  digitalWrite(led, LOW);  
}

void loop () {

  ether.packetLoop(ether.packetReceive());
  if(motionCondition == true){
    if(digitalRead(trigger) == HIGH){
      //REQUEST HERE
      ether.browseUrl(PSTR("/"), "?txrelay=opened", website, my_callback);   
      //END OF REQUEST
      digitalWrite(led, HIGH);
      Serial.println("Motion detected!");
      delay(1000);
      restoreCondition = true;
      motionCondition = false;      
    }
  }

  if(restoreCondition == true){
    if(digitalRead(trigger) == LOW){
      //REQUEST HERE
      ether.browseUrl(PSTR("/"), "?txrelay=closed", website, my_callback);   
      //END OF REQUEST
      digitalWrite(led, LOW);
      Serial.println("Sensor restored!");
      delay(1000);
      restoreCondition = false;
      motionCondition = true;
    }
  }
}
