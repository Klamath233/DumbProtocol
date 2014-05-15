#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define BUFFER_SIZE 4096

void run() {
	struct sockaddr_in s_addr, c_addr;
	int socketfd;
	char recv_buf[BUFFER_SIZE], send_buf[BUFFER_SIZE];
	socklen_t addr_len = sizeof(c_addr);
	memset(&s_addr.sin_zero, 0, sizeof(s_addr.sin_zero));
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = INADDR_ANY;
	s_addr.sin_port = htons(12345);

	socketfd = socket(PF_INET, SOCK_DGRAM, 0);

	bind(socketfd, (const struct sockaddr *) &s_addr, sizeof(s_addr));

	while (1) {
		memset(recv_buf, 0, BUFFER_SIZE);
		memset(send_buf, 0, BUFFER_SIZE);
		int byte_recved = 0;
		byte_recved = recvfrom(socketfd, recv_buf, BUFFER_SIZE, 0, (struct sockaddr *) &c_addr, &addr_len);
		sprintf(send_buf, "Hello! We have received %d bytes from you:\n", byte_recved);
		sendto(socketfd, send_buf, BUFFER_SIZE, 0, (struct sockaddr *) &c_addr, addr_len);
		memcpy(send_buf, recv_buf, BUFFER_SIZE);
		sendto(socketfd, send_buf, BUFFER_SIZE, 0, (struct sockaddr *) &c_addr, addr_len);
	}
}

int main(int argc, char **argv)
{
	run();
	return 0;
}
