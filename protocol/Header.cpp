#include "Header.h"
#include "global.h"
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>

Header::Header(unsigned short src_port,
               unsigned short dst_port, 
			   unsigned seq, 
			   unsigned ack,
			   bool syn, bool rst, bool fin, bool chk)
{
	_ack = ack;
	_seq = seq;
	_srcPort = src_port;
	_dstPort = dst_port;
	_syn = syn;
	_rst = rst;
	_fin = fin;
	_chk = chk;
	_checkSum = 0;
}

Header::Header(const char *header) {
	_srcPort = ntohs(*((unsigned short *) header));
	_dstPort = ntohs(*((unsigned short *) header + 1));
	_seq = ntohl(*((unsigned *) header + 1));
	_ack = ntohl(*((unsigned *) header + 2));
	_syn = (bool) (*((unsigned *) header + 3) & 0x80000000);
	_rst = (bool) (*((unsigned *) header + 3) & 0x40000000);
	_fin = (bool) (*((unsigned *) header + 3) & 0x20000000);
	_chk = (bool) (*((unsigned *) header + 3) & 0x10000000);
	_checkSum = ntohs(*((unsigned short *) header + 7));
}

unsigned short Header::getSrcPort() {
	return _srcPort;
} 

unsigned short Header::getDstPort() {
	return _dstPort;
}

unsigned Header::getSEQ() {
	return _seq;
}

unsigned Header::getACK() {
	return _ack;
}

bool Header::isSYN() {
	return _syn;
}

bool Header::isRST() {
	return _rst;
}

bool Header::isFIN() {
	return _fin;
}

bool Header::isCHK() {
	return _chk;
}

unsigned short Header::getChecksum() {
	return _checkSum;
}

void Header::setChecksum(unsigned short chksum) {
	_checkSum = chksum;
}

// **SP****DP**
// ****SEQ*****
// ****ACK*****
// *SRFC**SUM**
const char *Header::getHeader() {
	char *header = (char *) malloc(HEADER_SIZE);
	bzero(header, HEADER_SIZE);
	*((unsigned short *) header) = htons(_srcPort);
	*((unsigned short *) header + 1) = htons(_dstPort);
	*((unsigned *) header + 1) = htonl(_seq);
	*((unsigned *) header + 2) = htonl(_ack);
	if (_syn) {
		*((unsigned *) header + 3) |= 0x80000000;
	}
	if (_rst) {
		*((unsigned *) header + 3) |= 0x40000000;
	}
	if (_fin) {
		*((unsigned *) header + 3) |= 0x20000000;
	}
	if (_chk) {
		*((unsigned *) header + 3) |= 0x10000000;
		*((unsigned short *) header + 7) = htons(_checkSum);
	}

	return header;
}
