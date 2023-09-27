/*
  NAME: arduino-ethernet
  AUTH: <oguzhan.ince@protonmail.com>
  DATE: 04/09/2023
  DESC: arduino ile ağ yönetimi

Derleme Aşaması;
  (base) macintosh ~ » arduino-cli compile  \
  --fqbn arduino:avr:uno  \
  --port /dev/cu.usbserial-1410  \
  --libraries /Users/macbook/Documents/arduino-ethernet/libs/  \
  --build-cache-path /Users/macbook/Documents/arduino-ethernet/build-cache/   \
  --export-binaries --warnings all  \
  --output-dir /Users/macbook/Documents/arduino-ethernet/bin/   \
  --upload  \
  --verify  \
  --verbose  \
  --clean \
  /Users/macbook/Documents/arduino-ethernet/arduino-ethernet.ino
*/

#include <SPI.h>
#include <SoftwareSerial.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <utility/w5100.h>
// #include "/Users/macbook/Documents/arduino-ethernet/libs/Ethernet/src/Ethernet.h"
// #include "/Users/macbook/Documents/arduino-ethernet/libs/Ethernet/src/EthernetUdp.h"

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
IPAddress destinationIP(10, 28, 28, 9);  // Replace with the IP address of your destination node
IPAddress sourceIP(10, 28, 28, 22);       // Replace with your Arduino's IP address

IPAddress targetIP(10, 28, 28, 9); // Target IP address for ARP request
IPAddress broadcastIP(10, 28, 28, 255); // Broadcast IP address

unsigned int sequenceNumber = 0;
unsigned int ethernetInitVal = 0;

// This is a workaround solution to the issue of the Ethernet library not having a proper raw socket implementation.
EthernetRAW raw(0);
EthernetUDP udp;
unsigned int localPort = 1; // Local port to listen on

void setup() {

  Ethernet.begin(sourceMAC, sourceIP);
  Serial.begin(9600);

  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Waiting command: a: sendArpRequestz, p: echoRequestReply");

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

}


void loop() {

  // give the Ethernet shield a second to initialize:
  delay(1000);

  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'a') {
      delay(1000);
      Serial.println("sendArpRequestz sending");
      sendArpRequestz(targetIP);
      Serial.println("sendArpRequestz sended");
      delay(1000);
    }
    if (input == 'p') {
      delay(1000);
      Serial.println("echoRequestReply sending");
      echoRequestReply();
      Serial.println("echoRequestReply sended");
      delay(1000);
    }
  }
}


void echoRequestReply() {

  // ETHERNET HEADER
  byte packetBuffer[48];      // Create an Ethernet packet buffer - send
  memcpy(packetBuffer, destinationMAC, 6);  // Destination MAC address
  memcpy(packetBuffer + 6, sourceMAC, 6);   // Source MAC address
  packetBuffer[12] = 0x08; // EtherType: IPv4 (0x0800) (0b00001000) (8) (IP packet)

  // IP HEADER
  // IHL: Internet Header Length (4 bits): Number of 32-bit words in the header
  // IHL = 5 (minimum value) (0b0101) (5 * 32 bits = 160 bits = 20 bytes)
  // High nibble: version, low nibble: header length in 32-bit words (5)
  packetBuffer[14] = 0x45; // Version (4), IHL (5)
  packetBuffer[15] = 0x00; // Type of Service (0) (DSCP + ECN) (0x00) (0b00000000) (best effort)
  packetBuffer[16] = 0x00; // Total Length (placeholder)      -
  packetBuffer[17] = 0x00; // Total Length (placeholder)      -
  packetBuffer[18] = 0x00; // Identification (placeholder)    -
  packetBuffer[19] = 0x00; // Identification (placeholder)    -
  packetBuffer[20] = 0x00; // Flags and Fragment Offset       ?
  packetBuffer[21] = 0x00; // Flags and Fragment Offset       ?
  packetBuffer[22] = 0x40; // TTL (64)
//packetBuffer[23] = 0x01; // Protocol: ICMP (1) (0x01) 
  packetBuffer[23] = 0xFF; // Protocol: RAW (255) (0xFF)
  packetBuffer[24] = 0x00; // Header Checksum (placeholder)   +
  packetBuffer[25] = 0x00; // Header Checksum (placeholder)   +
  memcpy(packetBuffer + 26, sourceIP.operator[](0), 4);      // Source IP address
  memcpy(packetBuffer + 30, destinationIP.operator[](0), 4); // Destination IP address

  // ICMP HEADER
  packetBuffer[34] = 0x08; // Type: ICMP Echo Request (8) (0x08)
  packetBuffer[35] = 0x00; // Code: 0 (0x00) is default for ICMP Echo Request (ping)
  packetBuffer[36] = 0x00; // Checksum (placeholder)          +
  packetBuffer[37] = 0x00; // Checksum (placeholder)          +
  packetBuffer[38] = 0x00; // Identifier (placeholder)        -
  packetBuffer[39] = 0x00; // Identifier (placeholder)        -
  packetBuffer[40] = 0x00; // Sequence Number (placeholder)   -
  packetBuffer[41] = 0x00; // Sequence Number (placeholder)   -
  
  // ICMP DATA (optional) - 32 bytes total (8 x 32 bits)
  packetBuffer[42] = 0x41; // A
  packetBuffer[43] = 0x6C; // l
  packetBuffer[44] = 0x6F; // o
  packetBuffer[45] = 0x68; // h
  packetBuffer[46] = 0x61; // a
  packetBuffer[47] = 0x21; // !

  // Calculate IP header checksum
  uint16_t ipChecksum = calculateChecksum(packetBuffer + 14, 20);
  packetBuffer[24] = ipChecksum >> 8;   // Header Checksum (high byte)
  packetBuffer[25] = ipChecksum & 0xFF; // Header Checksum (low byte)

  // Calculate ICMP header checksum
  uint16_t icmpChecksum = calculateChecksum(packetBuffer + 34, 8);
  packetBuffer[36] = icmpChecksum >> 8;   // Checksum (high byte)
  packetBuffer[37] = icmpChecksum & 0xFF; // Checksum (low byte)



  uint16_t _offset = 0;
  uint8_t sockindex = Ethernet.socketBegin(SnMR::IPRAW, 2);
  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.writeSnDIPR(sockindex, broadcastIP); // sockindex and targetIP
  W5100.writeSnDPORT(sockindex, 9); // sockindex and port
  SPI.endTransaction();
  uint16_t bytes_written = Ethernet.socketBufferData(sockindex, _offset, packetBuffer, 48);
  _offset += bytes_written;

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.execCmdSn(sockindex, Sock_SEND);

  while ( (W5100.readSnIR(sockindex) & SnIR::SEND_OK) != SnIR::SEND_OK ) {
    if (W5100.readSnIR(sockindex) & SnIR::TIMEOUT) {
      W5100.writeSnIR(sockindex, (SnIR::SEND_OK|SnIR::TIMEOUT));
      SPI.endTransaction();
      Serial.println("icmp timeout\n");
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

  // exit(0);
  sequenceNumber++; // Increment the sequence number for the next packet
  delay(1000); // Wait 3 second before sending the next packet
  // Ethernet.socketDisconnect(client);
}




void sendArpRequestz(IPAddress targetIP) {

  Serial.println("sendArpRequestz started");
  byte arpPacket[42]; // ARP packet size is 42 bytes
  delay(1000);
  // Ethernet header
  memset(arpPacket, 0xFF, 6); // Destination MAC: Broadcast
  memcpy(arpPacket + 6, sourceMAC, 6); // Source MAC: Our MAC address
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
  // ARP data
  memcpy(arpPacket + 22, sourceMAC, 6); // Sender MAC
  memcpy(arpPacket + 28, sourceIP.operator[](0), 4); // Sender IP
  memset(arpPacket + 32, 0x00, 6); // Target MAC: 0x00 (unknown)
  memcpy(arpPacket + 38, targetIP.operator[](0), 4); // Target IP
  Serial.println("sendArpRequestz arp data ok");
  delay(1000);


  // Send ARP packet
  // socket beginPacket MACRAW 0
  uint16_t _offset = 0;
  uint8_t sockindex = Ethernet.socketBegin(SnMR::MACRAW, 2);
  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.writeSnDIPR(sockindex, broadcastIP); // sockindex and targetIP
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
}