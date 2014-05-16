#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <sys/types.h>

class Socket {
public:
	Socket(int portno);
	void connect(const string &dst_ip, int dst_portno);
	void accept();
	ssize_t send(const char *buf, int len);
	ssize_t recv(char *buf, int len);
	void close();
private:
	bool _open;
	int _laskACK;
	int _nextSEQ;
	// A TIMER
	bool _isOpen;
	int _portno;
	struct sockaddr *_dst_addr;
	vector<char> _send_buffer;
	vector<char> _recv_buffer;
	Segment *_segm_head; // Link list head of segments.
};
#endif
