#ifndef __HEADER_H__
#define __HEADER_H__

class Header {
public:
	Header(unsigned short seq, unsigned short ack, unsigned src_port, unsigned dst_port,
	       bool syn, bool rst, bool fin);
	unsigned short getSrcPort();
	unsigned short getDstPort(); // Need Runhang's note.
	unsigned getSEQ();
	unsigned getACK();
	bool isSYN();
	bool isRST();
	bool isFIN();
	char *getHeader();

private:
	unsigned _ack;
	unsigned _seq;
	unsigned short _srcPort;
	unsigned short _dstPort;
	bool _syn;
	bool _rst;
	bool _fin;
};
#endif
