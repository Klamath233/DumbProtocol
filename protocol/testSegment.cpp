#include "Segment.h"
#include "Header.h"
#include <iostream>

using namespace std;

int main(void) {
	Header *header = new Header(0, 1, 100, 101, 0, 0, 0, 0);
	char data[] = {'H', 'E', 'L', 'L', 'O', '\0'};
	Segment *segment = new Segment(header, data, 6);
	cout << segment->segment() << endl;
}
