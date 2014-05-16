#ifndef __SERVERSOCKET_H__
#define __SERVERSOCKET_H__
#include "Socket.h"

class ServerSocket {
public:
	ServerSocket(int port);
	Socket& accept();
	void close();

private:
	struct sockaddr this_addr;
	bool _open;
};
#endif
