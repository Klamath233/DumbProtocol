#include "Header.h"
#include <iostream>
#include <cstdio>

using namespace std;

int main(void) {
	Header *h = new Header(443, 80, 12, 34, true, false, false, true);
	cout << "source port: " << h->getSrcPort() << endl;
	cout << "destination port: " << h->getDstPort() << endl;
	cout << "sequence number: " << h->getSEQ() << endl;
	cout << "acknowledge number: " << h->getACK() << endl;
	cout << "syn: " << h->isSYN() << endl;
	cout << "rst: " << h->isRST() << endl;
	cout << "fin: " << h->isFIN() << endl;
	cout << "chk: " << h->isCHK() << endl;
	h->setChecksum(20);
	cout << "checksum: " << h->getChecksum() << endl;
	cout << endl;
	Header *hh = new Header(h->getHeader());
	cout << "source port: " << hh->getSrcPort() << endl;
	cout << "destination port: " << hh->getDstPort() << endl;
	cout << "sequence number: " << hh->getSEQ() << endl;
	cout << "acknowledge number: " << hh->getACK() << endl;
	cout << "syn: " << hh->isSYN() << endl;
	cout << "rst: " << hh->isRST() << endl;
	cout << "fin: " << hh->isFIN() << endl;
	cout << "chk: " << hh->isCHK() << endl;
	cout << "checksum: " << h->getChecksum() << endl;
	return 0;
}
