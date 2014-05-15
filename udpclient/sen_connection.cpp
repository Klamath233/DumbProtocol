#include "sen_connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cstring>

using namespace std;

#define BUFSIZE 1024

bool debug = true;
void RunSender (string ipaddress, int portNumber, int windowSize, float pLossPoss, float pCoruPoss)
{
	if(debug)
	{
		cerr << "ip address: " << ipaddress << endl;
		cerr << "port number: " << portNumber << endl;
		cerr << "window size: " << windowSize << endl;
		cerr << "packet loss possiblity: " << pLossPoss << endl;
		cerr << "packet corruption possibility: " << pCoruPoss << endl;
	}
	
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	char buf[BUFSIZE];
	char revbuf[BUFSIZE];
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sockfd < 0)
	{
		cerr << "cannot open socket\n";
		exit(1);
	}

	// Initialize send buffer and receive buffer
	bzero((char*)buf, BUFSIZE);
	bzero((char*)revbuf, BUFSIZE);

	// Initialize server addr
	bzero((char*)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(ipaddress.c_str());//htonl((192 << 24) | (241 << 16) | (218 << 8) | 69);
	serveraddr.sin_port = htons((unsigned short)(portNumber));

	// Initialize client addr, it should be the same as server addr
	bzero((char*)&clientaddr, sizeof(clientaddr));

	socklen_t serverlen = (socklen_t)sizeof(serveraddr);
	socklen_t clientlen = (socklen_t)sizeof(clientaddr);

	while(1)
	{
		// Send message
		bzero((char*)buf, BUFSIZE);
		string sendit;
		getline(cin, sendit);
		memcpy(buf, sendit.c_str(), sendit.size());
		printf("Sending: %s\n", buf);

		int sentbyte = sendto(sockfd, buf, strlen(buf), 0,
													(struct sockaddr*)&serveraddr, serverlen);
		if (sentbyte < 0)
			continue;

		// Prepare to receive
		bzero((char*)revbuf, BUFSIZE);
		int revbyte = recvfrom( sockfd, revbuf, BUFSIZE, 0, 
		                        (struct sockaddr*)&clientaddr, &clientlen);
		if (revbyte < 0)
		{
			cerr << "Error when receiving" << endl;
		}
		printf("Receive from server: %s\n", revbuf);

	}




	return;
}




































