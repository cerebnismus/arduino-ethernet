/*
  NAME: arduino-ethernet
  AUTH: <oguzhan.ince@protonmail.com>
  DATE: 04/09/2023
  DESC: arduino ile ağ yönetimi
*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <utility/w5100.h>


// calculate checksum for the IP header and ICMP header (if applicable) (RFC 1071)
uint16_t calculateChecksum(const byte* data, size_t length) {
  uint32_t sum = 0;
  uint16_t* ptr = (uint16_t*)data;
  while (length > 1) {
    sum += *ptr++;
    length -= 2;
  }
  if (length == 1) {sum += *(uint8_t*)ptr;}
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  return ~sum;
}


// MAC addresses must be unique on the LAN and can be assigned by the user or generated here randomly.
byte destinationMAC[] = {0xAC, 0xBC, 0x32, 0x9B, 0x28, 0x67}; // Replace with your Router's MAC address
byte sourceMAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};      // Replace with your Arduino's MAC address

// IP addresses are dependent on your local network.
IPAddress destinationIP(10, 28, 28, 9);   // IP address of your destination node
IPAddress wanDestinationIP(8, 8, 8, 8);       // Arduino's IP address
IPAddress sourceIP(10, 28, 28, 22);       // Arduino's IP address
IPAddress broadcastIP(10, 28, 28, 255);   // Broadcast IP address


void setup() {
  Ethernet.begin(sourceMAC, sourceIP);
  Serial.begin(9600);
  delay(1000);

  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());
  delay(1000);

  optionMenu();

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}


void loop() {
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'a') {
      delay(1000);
      sendArpRequestz(destinationIP); // Send ARP request for lan IP
    //sendArpRequestz(wanDestinationIP); // Send ARP request for wan IP
    //sendArpRequestz(broadcastIP); // Send ARP announcement
    }
    else if (input == 'p') {
      delay(1000);
    //sendEchoRequestz(destinationIP);     // Send ICMP echo request to lan IP
      sendEchoRequestz(wanDestinationIP);  // Send ICMP echo request to wan IP
    //sendEchoRequestz(broadcastIP);       // ?
    }
    else if (input == 'q') {
      delay(1000);
      Serial.println("Quitting");
      exit(0);
    }
    else {
      Serial.println("Invalid option");
      optionMenu();
    }
  }
}

// option function
void optionMenu() {
  Serial.println("  Options  ");
  Serial.println(" --------- ");
  Serial.println("  a : ping ARP   ");
  Serial.println("  p : ping ICMP  ");
  Serial.println("  t : traceroute ");
  Serial.println("  s : portscan   ");
  Serial.println("  q : quit       ");
}


void sendEchoRequestz(IPAddress destinationIP_echo) {
  Serial.println("sendEchoRequestz started");
  byte echoPacket[48]; // ICMP packet size is 48 bytes
  delay(1000);
  // Ethernet header
  memcpy(echoPacket, destinationMAC, 6);  // Destination MAC address
  memcpy(echoPacket + 6, sourceMAC, 6);   // Source MAC address
  echoPacket[12] = 0x08; // EtherType: IPv4
  echoPacket[13] = 0x00;
  Serial.println("sendEchoRequestz ethernet header ok");
  delay(1000);
  // IP header
  echoPacket[14] = 0x45; // Version (4), IHL (5)              +
  echoPacket[15] = 0x00; // Type of Service (0) (DSCP + ECN)  +
  echoPacket[16] = 0x00; // Total Length (48)                 +
  echoPacket[17] = 0x30; //                                   +
  echoPacket[18] = 0x00; // Identification (placeholder)      +
  echoPacket[19] = 0x00; //                                   +
  echoPacket[20] = 0x00; // Flags                             +
  echoPacket[21] = 0x00; // Fragment Offset                   +
  echoPacket[22] = 0x40; // TTL (64)                          +
  echoPacket[23] = 0x01; // Protocol: ICMP (1) (0x01)         +
//echoPacket[23] = 0xFF; // Protocol: RAW (255) (0xFF)        +
  echoPacket[24] = 0x00; // Header Checksum (placeholder)     +
  echoPacket[25] = 0x00; // Header Checksum (placeholder)     +
  memcpy(echoPacket + 26, (uint8_t *)&sourceIP+2, 4); //      +
  memcpy(echoPacket + 30, (uint8_t *)&destinationIP_echo+2, 4); // +
  Serial.println("sendEchoRequestz ip header ok");
  // ICMP HEADER
  echoPacket[34] = 0x08; // Type: 8 (0x08) Echo Request       +
  echoPacket[35] = 0x00; // Code: 0 (0x00)                    +
  echoPacket[36] = 0x00; // Checksum (placeholder)            +
  echoPacket[37] = 0x00; // Checksum (placeholder)            +
  echoPacket[38] = 0x00; // Identifier (placeholder)          -
  echoPacket[39] = 0x00; // Identifier (placeholder)          -
  echoPacket[40] = 0x00; // Sequence Number (placeholder)     -
  echoPacket[41] = 0x00; // Sequence Number (placeholder)     -
  Serial.println("sendEchoRequestz icmp header ok");

  // ICMP data (optional)
  echoPacket[42] = 0x41; // A
  echoPacket[43] = 0x6C; // l
  echoPacket[44] = 0x6F; // o
  echoPacket[45] = 0x68; // h
  echoPacket[46] = 0x61; // a
  echoPacket[47] = 0x21; // !

  // Calculate IP header checksum
  uint16_t ipChecksum = calculateChecksum(echoPacket + 14, 20);
  echoPacket[24] = ipChecksum >> 8;   // Header Checksum (high byte)
  echoPacket[25] = ipChecksum & 0xFF; // Header Checksum (low byte)

  // Calculate ICMP header checksum
  uint16_t icmpChecksum = calculateChecksum(echoPacket + 34, 8);
  echoPacket[36] = icmpChecksum >> 8;   // Checksum (high byte)
  echoPacket[37] = icmpChecksum & 0xFF; // Checksum (low byte)

  // Send ICMP packet
  uint16_t _offset = 0;
  uint8_t sockindex = Ethernet.socketBegin(SnMR::MACRAW, 2);

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.writeSnDIPR(sockindex, destinationIP_echo); // sockindex and destinationIP_echo
  W5100.writeSnDPORT(sockindex, 7); // sockindex and port
  SPI.endTransaction();
  uint16_t bytes_written = Ethernet.socketBufferData(sockindex, _offset, echoPacket, 48);
  _offset += bytes_written;

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.execCmdSn(sockindex, Sock_SEND);

  while ( (W5100.readSnIR(sockindex) & SnIR::SEND_OK) != SnIR::SEND_OK ) {
    if (W5100.readSnIR(sockindex) & SnIR::TIMEOUT) {
      W5100.writeSnIR(sockindex, (SnIR::SEND_OK|SnIR::TIMEOUT));
      SPI.endTransaction();
      Serial.println("ICMP Echo Request timeout.\n");
    }
    SPI.endTransaction();
    yield();
    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  }
  W5100.writeSnIR(sockindex, SnIR::SEND_OK);
  SPI.endTransaction();

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.execCmdSn(sockindex, Sock_CLOSE);
  SPI.endTransaction();
  Serial.println("ICMP Echo Request packet sent.");
}


void sendArpRequestz(IPAddress destinationIP_arp) {
  Serial.println("sendArpRequestz started");
  byte arpPacket[42]; // ARP packet size is 42 bytes
  delay(1000);
  // Ethernet header
  memset(arpPacket, 0xFF, 6);          // Destination MAC Broadcast
  memcpy(arpPacket + 6, sourceMAC, 6); // Source MAC address
  arpPacket[12] = 0x08; // Ethertype: ARP
  arpPacket[13] = 0x06;
  Serial.println("sendArpRequestz ethernet header ok");
  delay(1000);
  // ARP header
  arpPacket[14] = 0x00; // Hardware type: Ethernet
  arpPacket[15] = 0x01;
  arpPacket[16] = 0x08; // Protocol type: ARP
  arpPacket[17] = 0x00;
  arpPacket[18] = 0x06; // Hardware address length // Ethernet address length is 6.
  arpPacket[19] = 0x04; // Protocol address length // IPv4 address length is 4
  arpPacket[20] = 0x00; // Opcode: Request
  arpPacket[21] = 0x01; // 1 for request, 2 for reply.
  Serial.println("sendArpRequestz arp header ok");
  delay(1000);

  /*
  Source ve Targer IP addresses mustbe +2 offset 
  why ? I dont know but it works

    when dont use +2 offset then ARP packet is like this:
    Sender MAC address: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
    Sender IP address: 84.1.10.28 (84.1.10.28)

    when use +2 offset then ARP packet is like this:
    Target MAC address: 00:00:00_00:00:00 (00:00:00:00:00:00)
    Target IP address: 10.28.28.9 (10.28.28.9)
  */

  memcpy(arpPacket + 22, sourceMAC, 6); // Sender MAC
  memcpy(arpPacket + 28, (uint8_t *)&sourceIP+2, 4); // Sender IP
  // Target MAC: 0x00 (for unknown)
  // Target MAC: 0xFF (for broadcast)
  memset(arpPacket + 32, 0xFF, 6); 
  memcpy(arpPacket + 38, (uint8_t *)&destinationIP_arp+2, 4);
  Serial.println("sendArpRequestz arp data ok");
  delay(1000);

  // Send ARP packet
  // socket beginPacket MACRAW 0
  uint16_t _offset = 0;
  uint8_t sockindex = Ethernet.socketBegin(SnMR::MACRAW, 2);
  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.writeSnDIPR(sockindex, destinationIP_arp); // sockindex and destinationIP_arp
  W5100.writeSnDPORT(sockindex, 9); // sockindex and port
  SPI.endTransaction();
  uint16_t bytes_written = Ethernet.socketBufferData(sockindex, _offset, arpPacket, 42);
  _offset += bytes_written;

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.execCmdSn(sockindex, Sock_SEND);

  while ( (W5100.readSnIR(sockindex) & SnIR::SEND_OK) != SnIR::SEND_OK ) {
    if (W5100.readSnIR(sockindex) & SnIR::TIMEOUT) {
      W5100.writeSnIR(sockindex, (SnIR::SEND_OK|SnIR::TIMEOUT));
      SPI.endTransaction();
      Serial.println("ARP timeout\n");
    }
    SPI.endTransaction();
    yield();
    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  }
  W5100.writeSnIR(sockindex, SnIR::SEND_OK);
  SPI.endTransaction();

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.execCmdSn(sockindex, Sock_CLOSE);
  SPI.endTransaction();
  Serial.println("ARP requestz sent");
  delay(1000);
}

/*
ARP Request:
Frame 5147: 60 bytes on wire (480 bits), 60 bytes captured (480 bits) on interface en0, id 0
    Section number: 1
    Interface id: 0 (en0)
    Encapsulation type: Ethernet (1)
    Arrival Time: Sep 28, 2023 03:02:32.528096000 +03
    [Time shift for this packet: 0.000000000 seconds]
    Epoch Time: 1695859352.528096000 seconds
    [Time delta from previous captured frame: 1.478242000 seconds]
    [Time delta from previous displayed frame: 20.989089000 seconds]
    [Time since reference or first frame: 258.845140000 seconds]
    Frame Number: 5147
    Frame Length: 60 bytes (480 bits)
    Capture Length: 60 bytes (480 bits)
    [Frame is marked: False]
    [Frame is ignored: False]
    File Offset: 2553296 (0x26f5d0)
    [Protocols in frame: eth:ethertype:arp]
    [Coloring Rule Name: ARP]
    [Coloring Rule String: arp]
Ethernet II, Src: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed), Dst: Broadcast (ff:ff:ff:ff:ff:ff)
    Destination: Broadcast (ff:ff:ff:ff:ff:ff)
        Address: Broadcast (ff:ff:ff:ff:ff:ff)
        .... ..1. .... .... .... .... = LG bit: Locally administered address (this is NOT the factory default)
        .... ...1 .... .... .... .... = IG bit: Group address (multicast/broadcast)
    Source: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
        Address: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
        .... ..1. .... .... .... .... = LG bit: Locally administered address (this is NOT the factory default)
        .... ...0 .... .... .... .... = IG bit: Individual address (unicast)
    Type: ARP (0x0806)
    Padding: 000000000000000000000000000000000000
Address Resolution Protocol (request)
    Hardware type: Ethernet (1)
    Protocol type: IPv4 (0x0800)
    Hardware size: 6
    Protocol size: 4
    Opcode: request (1)
    Sender MAC address: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
    Sender IP address: 10.28.28.22 (10.28.28.22)
    Target MAC address: 00:00:00_00:00:00 (00:00:00:00:00:00)
    Target IP address: macintosh.local (10.28.28.9)



ARP Reply:
Frame 5148: 42 bytes on wire (336 bits), 42 bytes captured (336 bits) on interface en0, id 0
    Section number: 1
    Interface id: 0 (en0)
    Encapsulation type: Ethernet (1)
    Arrival Time: Sep 28, 2023 03:02:32.528186000 +03
    [Time shift for this packet: 0.000000000 seconds]
    Epoch Time: 1695859352.528186000 seconds
    [Time delta from previous captured frame: 0.000090000 seconds]
    [Time delta from previous displayed frame: 0.000090000 seconds]
    [Time since reference or first frame: 258.845230000 seconds]
    Frame Number: 5148
    Frame Length: 42 bytes (336 bits)
    Capture Length: 42 bytes (336 bits)
    [Frame is marked: False]
    [Frame is ignored: False]
    File Offset: 2553388 (0x26f62c)
    [Protocols in frame: eth:ethertype:arp]
    [Coloring Rule Name: ARP]
    [Coloring Rule String: arp]
Ethernet II, Src: macintosh.local (ac:bc:32:9b:28:67), Dst: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
    Destination: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
        Address: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
        .... ..1. .... .... .... .... = LG bit: Locally administered address (this is NOT the factory default)
        .... ...0 .... .... .... .... = IG bit: Individual address (unicast)
    Source: macintosh.local (ac:bc:32:9b:28:67)
        Address: macintosh.local (ac:bc:32:9b:28:67)
        .... ..0. .... .... .... .... = LG bit: Globally unique address (factory default)
        .... ...0 .... .... .... .... = IG bit: Individual address (unicast)
    Type: ARP (0x0806)
Address Resolution Protocol (reply)
    Hardware type: Ethernet (1)
    Protocol type: IPv4 (0x0800)
    Hardware size: 6
    Protocol size: 4
    Opcode: reply (2)
    Sender MAC address: macintosh.local (ac:bc:32:9b:28:67)
    Sender IP address: macintosh.local (10.28.28.9)
    Target MAC address: de:ad:be:ef:fe:ed (de:ad:be:ef:fe:ed)
    Target IP address: 10.28.28.22 (10.28.28.22)
*/