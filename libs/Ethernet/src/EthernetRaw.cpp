/* Copyright 2018 Paul Stoffregen
 * Copyright 2023 Oguzhan Ince
 * Arduino RAW Socket eXtension for WizNet5100-based Ethernet shield
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "Ethernet.h"
#include "Dns.h"
#include "utility/w5100.h"

uint16_t EthernetRAW::server_port[MAX_SOCK_NUM];
// uint16_t EthernetRAW::sockindex = MAX_SOCK_NUM;


/* Start Ethernet IPRAW socket */
void EthernetRAW::begin()
{
	sockindex = Ethernet.socketBegin(SnMR::MACRAW, 0);
  //SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  //W5100.writeSnMR(0, 0x04); // Set socket IPRAW mode IPRAW  = 0x03; MACRAW = 0x04
  //W5100.writeSnPROTO(0, 0xFF); // Set PROTO register to ICMP 1 (0x01) or RAW 255 (0xFF)
  //W5100.execCmdSn(0, Sock_OPEN);
  //SPI.endTransaction();
}


EthernetClient EthernetRAW::available()
{
	bool listening = false;
	uint8_t sockindex = MAX_SOCK_NUM;
	uint8_t chip, maxindex=MAX_SOCK_NUM;

	chip = W5100.getChip();
	if (!chip) return EthernetClient(MAX_SOCK_NUM);
#if MAX_SOCK_NUM > 4
	if (chip == 51) maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i=0; i < maxindex; i++) {
		if (server_port[i] == _port) {
			uint8_t stat = Ethernet.socketStatus(i);
			if (stat == SnSR::ESTABLISHED || stat == SnSR::CLOSE_WAIT) {
				if (Ethernet.socketRecvAvailable(i) > 0) {
					sockindex = i;
				} else {
					// remote host closed connection, our end still open
					if (stat == SnSR::CLOSE_WAIT) {
						Ethernet.socketDisconnect(i);
						// status becomes LAST_ACK for short time
					}
				}
			} else if (stat == SnSR::LISTEN) {
				listening = true;
			} else if (stat == SnSR::CLOSED) {
				server_port[i] = 0;
			}
		}
	}
	if (!listening) begin();
	return EthernetClient(sockindex);
}


EthernetRAW::operator bool()
{
	uint8_t maxindex=MAX_SOCK_NUM;
#if MAX_SOCK_NUM > 4
	if (W5100.getChip() == 51) maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i=0; i < maxindex; i++) {
		if (server_port[i] == _port) {
			if (Ethernet.socketStatus(i) == SnSR::LISTEN) {
				return true; // server is listening for incoming clients
			}
		}
	}
	return false;
}

#if 0
void EthernetRAW::statusreport()
{
	Serial.printf("EthernetRAW, port=%d\n", _port);
	for (uint8_t i=0; i < MAX_SOCK_NUM; i++) {
		uint16_t port = server_port[i];
		uint8_t stat = Ethernet.socketStatus(i);
		const char *name;
		switch (stat) {
			case 0x00: name = "CLOSED"; break;
			case 0x13: name = "INIT"; break;
			case 0x14: name = "LISTEN"; break;
			case 0x15: name = "SYNSENT"; break;
			case 0x16: name = "SYNRECV"; break;
			case 0x17: name = "ESTABLISHED"; break;
			case 0x18: name = "FIN_WAIT"; break;
			case 0x1A: name = "CLOSING"; break;
			case 0x1B: name = "TIME_WAIT"; break;
			case 0x1C: name = "CLOSE_WAIT"; break;
			case 0x1D: name = "LAST_ACK"; break;
			case 0x22: name = "UDP"; break;
			case 0x32: name = "IPRAW"; break;
			case 0x42: name = "MACRAW"; break;
			case 0x5F: name = "PPPOE"; break;
			default: name = "???";
		}
		int avail = Ethernet.socketRecvAvailable(i);
		Serial.printf("  %d: port=%d, status=%s (0x%02X), avail=%d\n",
			i, port, name, stat, avail);
	}
}
#endif

size_t EthernetRAW::write(uint8_t b)
{
	return write(&b, 1);
}

size_t EthernetRAW::write(const uint8_t *buffer, size_t size)
{
	// Assuming socket 0 is used

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	uint16_t txWriteAddr = W5100.readSnTX_WR(0); // Get the current write address
	W5100.write(txWriteAddr, buffer, size); // Write your packet to the TX buffer
	W5100.writeSnTX_WR(0, txWriteAddr + size); // update the TX write pointer
	W5100.execCmdSn(0, Sock_SEND);
  SPI.endTransaction();

}

/* Release any resources being used by this EthernetUDP instance */
void EthernetRAW::stop()
{
	if (sockindex < MAX_SOCK_NUM) {
		Ethernet.socketClose(sockindex);
		sockindex = MAX_SOCK_NUM;
	}
}