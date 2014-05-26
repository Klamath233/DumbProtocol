#ifndef __SEGMENT_H__
#define __SEGMENT_H__
#include <sys/types.h>
#include "Header.h"

class Segment {
public:
	Segment(const Header *header, const char *data, size_t len);
	const char *data(size_t &len);
	const Header *header();
	const char *segment();
private:
	const Header *_header;
	const char *_data;
	size_t _len;
};

#endif
