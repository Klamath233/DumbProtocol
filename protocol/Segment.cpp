#include "Segment.h"
#include "Header.h"
#include <cstdlib>
#include <cstring>
#include "global.h"

Segment::Segment(const Header *header, const char *data, size_t len) {
	_header = header;
	_data = data;
	_len = len;
}

const char *Segment::data(size_t &len) {
	len = _len;
	return _data;
}

const Header *Segment::header() {
	return _header;
}

const char *Segment::segment() {
	char *ret = (char *) malloc(SEG_SIZE);
	bzero(ret, SEG_SIZE);
	memcpy(ret, _header, HEADER_SIZE);
	memcpy(ret + HEADER_SIZE, _data, _len);
	return ret;
}
