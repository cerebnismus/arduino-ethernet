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


// Function to calculate checksum (byte-swapped) (Big Endian)
uint16_t icmpChecksum(uint8_t *data, uint16_t length) {
  uint32_t sum = 0; // Sum up 16-bit words
  for (int i = 0; i < length; i += 2) {
    uint16_t word = (data[i] << 8) + data[i + 1];
    sum += word;
  }
  while (sum >> 16) {  // Add carry bits
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
  uint16_t checksum = ~sum; // One's complement
  checksum = (checksum >> 8) | (checksum << 8); // Swap bytes
  return checksum;
}

// MAC addresses must be unique on the LAN and can be assigned by the user or generated here randomly.
byte destinationMAC[] = {0x00, 0x1C, 0xA8, 0xDE, 0xAD, 0x05}; // Replace with your Router's MAC address
byte sourceMAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};      // Replace with your Arduino's MAC address

// IP addresses are dependent on your local network.
IPAddress destinationIP(10, 28, 28, 9);   // IP address of your destination node
IPAddress wanDestinationIP(8, 8, 8, 8);   // Arduino's IP address
IPAddress sourceIP(10, 28, 28, 22);       // Arduino's IP address
IPAddress broadcastIP(10, 28, 28, 255);   // Broadcast IP address


void setup() {
  Ethernet.begin(sourceMAC, sourceIP);
  Serial.begin(9600);
  delay(1000);

  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());
  delay(1000);

  Serial.println("  Options  ");
  Serial.println(" --------- ");
  Serial.println("  a : ARP request   ");
  Serial.println("  b : ARP broadcast ");
  Serial.println("  l : LAN ping      ");
  Serial.println("  w : WAN ping      ");
  Serial.println("  t : traceroute    ");
  Serial.println("  p : portscan      ");
  Serial.println("  q : quit          ");

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
    }
    else if (input == 'b') {
      delay(1000);
      sendArpRequestz(broadcastIP); // Send ARP announcement
    }
    else if (input == 'l') {
      delay(1000);
      sendEchoRequestz(destinationIP);     // Send ICMP echo request to lan IP
    }
    else if (input == 'w') {
      delay(1000);
      sendEchoRequestz(wanDestinationIP);  // Send ICMP echo request to wan IP
    }
    else if (input == 't') {
      delay(1000);
      sendTraceRequestz(wanDestinationIP);
      // TODO
    }
    else if (input == 'p') {
      delay(1000);
      // Serial.println("portscan");
      // TODO
    }
    else if (input == 'q') {
      delay(1000);
      Serial.println("Quitting");
      exit(0);
    }
    else {
      Serial.println("Invalid option");
    }
  }
}


void sendEchoRequestz(IPAddress destinationIP_echo) {
  Serial.println("sendEchoRequestz started");
  byte echoPacket[42]; // ICMP packet size is 42 bytes
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
  echoPacket[16] = 0x00; // Total Length (46)                 +
  echoPacket[17] = 0x2E; //                                   +
  echoPacket[18] = 0x01; // Identification (placeholder)      +
  echoPacket[19] = 0x01; //                                   +
  echoPacket[20] = 0x00; // Flags                             +
  echoPacket[21] = 0x00; // Fragment Offset                   +
  echoPacket[22] = 0x40; // TTL (64)                          +
  echoPacket[23] = 0x01; // Protocol: ICMP (1) (0x01)         +
//echoPacket[23] = 0xFF; // Protocol: RAW (255) (0xFF)        +
  echoPacket[24] = 0x00; // Header Checksum (placeholder)     +
  echoPacket[25] = 0x00; // Header Checksum (placeholder)     +
  memcpy(echoPacket + 26, (uint8_t *)&sourceIP+2, 4); //      +
  memcpy(echoPacket + 30, (uint8_t *)&destinationIP_echo+2, 4);
  echoPacket[34] = 0x08; // Type: 8 (0x08) Echo Request       +
  echoPacket[35] = 0x00; // Code: 0 (0x00)                    +
  echoPacket[36] = 0x00; // Checksum (X)            +
  echoPacket[37] = 0x00; // Checksum (X)            +
  echoPacket[38] = 0x01; // Identification (placeholder)      +
  echoPacket[39] = 0x01; //                                   +
  echoPacket[40] = 0x00; // Sequence Number (placeholder)     +
  echoPacket[41] = 0x00; //                                   +

  // IP header checksum
  uint16_t checksum_ip = icmpChecksum(echoPacket + 14, 20);
  echoPacket[24] = checksum_ip & 0xFF; // Checksum (low byte)
  echoPacket[25] = checksum_ip >> 8;   // Checksum (high byte)
  delay(1000);

  // ICMP header checksum (calculate with payload)
  uint16_t checksum_icmp = icmpChecksum(echoPacket + 34, 8);
  echoPacket[36] = checksum_icmp & 0xFF; // Checksum (low byte)
  echoPacket[37] = checksum_icmp >> 8;   // Checksum (high byte)

  // Send ICMP packet
  uint16_t _offset = 0;
  uint8_t sockindex = Ethernet.socketBegin(SnMR::MACRAW, 2);

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.writeSnDIPR(sockindex, destinationIP_echo); // sockindex and destinationIP_echo
  W5100.writeSnDPORT(sockindex, 7); // sockindex and port
  SPI.endTransaction();
  uint16_t bytes_written = Ethernet.socketBufferData(sockindex, _offset, echoPacket, 42);
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
  // Serial.println("ICMP Echo Request packet sent.");

  // Read packet
  while ( (W5100.readSnIR(sockindex) & SnIR::RECV) != SnIR::RECV ) {
    if (W5100.readSnIR(sockindex) & SnIR::TIMEOUT) {
      W5100.writeSnIR(sockindex, SnIR::RECV);
      SPI.endTransaction();
      Serial.println("ICMP Echo Request timeout.\n");
    }
    SPI.endTransaction();
    yield();
    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  }
  W5100.writeSnIR(sockindex, SnIR::RECV);

  // Read packet size
  uint16_t dataSize = W5100.readSnRX_RSR(sockindex);
  if (dataSize > 0) {
    uint8_t buffer[dataSize];

    // W5100.readSn(sockindex, (uint8_t *)&destinationIP_echo+2, buffer, dataSize);
    Ethernet.socketRecv(sockindex, buffer, dataSize);
    printPacket(buffer, dataSize);
  }
  delay(1000);

  // Close socket
  W5100.execCmdSn(sockindex, Sock_CLOSE);
  SPI.endTransaction();
}


void printPacket(uint8_t *buffer, uint16_t size) {
  for (uint16_t i = 0; i < size; ++i) {
    Serial.print(i, DEC);
    Serial.println(buffer[i], HEX);
  }
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



void sendTraceRequestz(IPAddress destinationIP_echo) {
  Serial.println();
  Serial.print("traceroute to (");
  Serial.print(destinationIP_echo);
  Serial.print("), 30 hops max, 42 byte packets");
  Serial.println();
  delay(1000);
  // loop for traceroute ttl value
  for (int i = 1; i < 30; i++) {
    delay(100);
    byte echoPacket[42]; // ICMP packet size is 42 bytes
    memcpy(echoPacket, destinationMAC, 6);  // Destination MAC address
    memcpy(echoPacket + 6, sourceMAC, 6);   // Source MAC address
    echoPacket[12] = 0x08; // EtherType: IPv4
    echoPacket[13] = 0x00;
    delay(100);
    // IP header
    echoPacket[14] = 0x45; // Version (4), IHL (5)              +
    echoPacket[15] = 0x00; // Type of Service (0) (DSCP + ECN)  +
    echoPacket[16] = 0x00; // Total Length (46)                 +
    echoPacket[17] = 0x2E; //                                   +
    echoPacket[18] = 0x01; // Identification (placeholder)      +
    echoPacket[19] = 0x01; //                                   +
    echoPacket[20] = 0x00; // Flags                             +
    echoPacket[21] = 0x00; // Fragment Offset                   +
    echoPacket[22] = i;    // TTL (MAX HOP 30)                  +
    echoPacket[23] = 0x01; // Protocol: ICMP (1) (0x01)         +
  //echoPacket[23] = 0xFF; // Protocol: RAW (255) (0xFF)        +
    echoPacket[24] = 0x00; // Header Checksum (placeholder)     +
    echoPacket[25] = 0x00; // Header Checksum (placeholder)     +
    memcpy(echoPacket + 26, (uint8_t *)&sourceIP+2, 4); //      +
    memcpy(echoPacket + 30, (uint8_t *)&destinationIP_echo+2, 4);
    echoPacket[34] = 0x08; // Type: 8 (0x08) Echo Request       +
    echoPacket[35] = 0x00; // Code: 0 (0x00)                    +
    echoPacket[36] = 0x00; // Checksum (X)            +
    echoPacket[37] = 0x00; // Checksum (X)            +
    echoPacket[38] = 0x01; // Identification (placeholder)      +
    echoPacket[39] = 0x01; //                                   +
    echoPacket[40] = 0x00; // Sequence Number (placeholder)     +
    echoPacket[41] = 0x00; //                                   +

    // IP header checksum
    uint16_t checksum_ip = icmpChecksum(echoPacket + 14, 20);
    echoPacket[24] = checksum_ip & 0xFF; // Checksum (low byte)
    echoPacket[25] = checksum_ip >> 8;   // Checksum (high byte)
    delay(100);

    // ICMP header checksum (calculate with payload)
    uint16_t checksum_icmp = icmpChecksum(echoPacket + 34, 8);
    echoPacket[36] = checksum_icmp & 0xFF; // Checksum (low byte)
    echoPacket[37] = checksum_icmp >> 8;   // Checksum (high byte)

    // Send ICMP packet
    uint16_t _offset = 0;
    uint8_t sockindex = Ethernet.socketBegin(SnMR::MACRAW, 2);

    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    W5100.writeSnDIPR(sockindex, destinationIP_echo); // sockindex and destinationIP_echo
    W5100.writeSnDPORT(sockindex, 7); // sockindex and port
    SPI.endTransaction();
    uint16_t bytes_written = Ethernet.socketBufferData(sockindex, _offset, echoPacket, 42);
    _offset += bytes_written;

    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    W5100.execCmdSn(sockindex, Sock_SEND);

    while ( (W5100.readSnIR(sockindex) & SnIR::SEND_OK) != SnIR::SEND_OK ) {
      if (W5100.readSnIR(sockindex) & SnIR::TIMEOUT) {
        W5100.writeSnIR(sockindex, (SnIR::SEND_OK|SnIR::TIMEOUT));
        SPI.endTransaction();
        // Serial.println("ICMP Echo Request timeout.\n");
      }
      SPI.endTransaction();
      yield();
      SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    }
    W5100.writeSnIR(sockindex, SnIR::SEND_OK);
    SPI.endTransaction();
    // Serial.println("ICMP Echo Request packet sent.");

    // Read packet
    while ( (W5100.readSnIR(sockindex) & SnIR::RECV) != SnIR::RECV ) {
      if (W5100.readSnIR(sockindex) & SnIR::TIMEOUT) {
        W5100.writeSnIR(sockindex, SnIR::RECV);
        SPI.endTransaction();
        // Serial.println("ICMP Echo Request timeout.\n");
      }
      SPI.endTransaction();
      yield();
      SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    }
    W5100.writeSnIR(sockindex, SnIR::RECV);

    // Read packet size
    uint16_t dataSize = W5100.readSnRX_RSR(sockindex);
    if (dataSize > 0) {
      uint8_t buffer[dataSize];
      delay(100);

      // W5100.readSn(sockindex, (uint8_t *)&destinationIP_echo+2, buffer, dataSize);
      Ethernet.socketRecv(sockindex, buffer, dataSize);
      // printPacket(buffer, dataSize);

      // buffer[36] = 11 and buffer[37] = 0 or buffer[36] = 0 and buffer[37] = 0
      if (buffer[36] == 11 && buffer[37] == 0 || buffer[36] == 0 && buffer[37] == 0) {
        delay(100);
        Serial.print(i, DEC);
        Serial.print(" ");
        delay(100);
        Serial.print(" Type/Code: ");
        Serial.print(buffer[36], DEC);
        Serial.print("/");
        Serial.print(buffer[37], DEC);
        Serial.print(" ");
        Serial.print(buffer[28], DEC);
        Serial.print(".");
        Serial.print(buffer[29], DEC);
        Serial.print(".");
        Serial.print(buffer[30], DEC);
        Serial.print(".");
        Serial.print(buffer[31], DEC);
        Serial.println();
        delay(100);
        if (buffer[28] == destinationIP_echo[0] && buffer[29] == destinationIP_echo[1] && buffer[30] == destinationIP_echo[2] && buffer[31] == destinationIP_echo[3]) {
          delay(100);
          Serial.println("traceroute completed");
          delay(1000);
          W5100.execCmdSn(sockindex, Sock_CLOSE);
          SPI.endTransaction();
          exit(0);
        }
      }
    }
    delay(100);

    // Close socket
    W5100.execCmdSn(sockindex, Sock_CLOSE);
    SPI.endTransaction();
  }
}