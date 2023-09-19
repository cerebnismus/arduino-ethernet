#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <ProtoThreads.h>

// Ethernet and SNMP settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
EthernetUDP udp;
unsigned int localPort = 162;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

// ProtoThreads contexts
struct pt ptSnmp, ptPing;  

// LED Indicator Pin
const int ledPin = 13;

// Enums for state machines
enum SNMPState { LISTENING, PROCESSING };
enum PINGState { PING_WAIT, PING_SEND };

// Current states
SNMPState snmpState = LISTENING;
PINGState pingState = PING_WAIT;

// Function to process SNMP data
void processSNMP(char *data) {
  // Add your SNMP processing code here
  // For now, simply printing the data
  Serial.println(data);
}

// Function to perform PING
void performPing() {
  // Add your PING code here
  // For now, simply printing a message
  Serial.println("Ping sent!");
}

// Simulated SNMP function
static int doSnmp(struct pt *pt) {
  PT_BEGIN(pt);

  while(1) {
    switch(snmpState) {
      case LISTENING:
        // Listen for incoming SNMP packets on port 162
        int packetSize = udp.parsePacket();
        if (packetSize) {
          udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
          snmpState = PROCESSING;
        }
        break;

      case PROCESSING:
        digitalWrite(ledPin, HIGH);  // LED ON
        processSNMP(packetBuffer); // Process SNMP data
        delay(100); // Simulate delay for processing; avoid using delay in actual applications
        digitalWrite(ledPin, LOW);  // LED OFF
        snmpState = LISTENING;
        break;
    }
    PT_YIELD(pt);
  }

  PT_END(pt);
}

// Simulated PING function
static int doPing(struct pt *pt) {
  PT_BEGIN(pt);

  while(1) {
    switch(pingState) {
      case PING_WAIT:
        // Wait before sending next PING
        delay(3000); // Simulate waiting
        pingState = PING_SEND;
        break;

      case PING_SEND:
        performPing(); // Perform PING operation
        pingState = PING_WAIT;
        break;
    }
    PT_YIELD(pt);
  }
  
  PT_END(pt);
}

void setup() {
  // Initialize Serial, Ethernet, ProtoThreads, and UDP
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  udp.begin(localPort);
  PT_INIT(&ptSnmp);
  PT_INIT(&ptPing);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Run the protothreads for SNMP and PING
  doSnmp(&ptSnmp);
  doPing(&ptPing);
}
