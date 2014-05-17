#ifndef __HEADER_H__
#define __HEADER_H__

class Header {
public:
	Header(unsigned short src_port, 
	       unsigned short dst_port, 
		   unsigned seq, 
		   unsigned ack,
	       bool syn, bool rst, bool fin, bool chk);
	Header(const char *header);
	unsigned short getSrcPort();
	unsigned short getDstPort();
	unsigned getSEQ();
	unsigned getACK();
	bool isSYN();
	bool isRST();
	bool isFIN();
	bool isCHK();
	unsigned short getChecksum();
	void setChecksum(unsigned short chksum);
	const char *getHeader();

private:
	unsigned _ack;
	unsigned _seq;
	unsigned short _srcPort;
	unsigned short _dstPort;
	unsigned short _checkSum;
	bool _syn;
	bool _rst;
	bool _fin;
	bool _chk; // Is checksum enabled? (not applicable at this point)
};
#endif
