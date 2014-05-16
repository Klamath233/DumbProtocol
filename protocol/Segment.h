#ifndef __SEGMENT_H__
#define __SEGMENT_H__
#include <sys/types.h>

class Segment {
public:
	Segment(const Header *header, const char *data, size_t len);
	const char *data(size_t &len);
	const Header *header();
	const char *segment();
private:
	Header *_header;
	char *_data;
	size_t _len;
};

#endif
