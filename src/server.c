#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <event2/event.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <time.h>

#define PACKET_SIZE 1024
#define HEADER_SIZE 8
#define DATA_SIZE (PACKET_SIZE - HEADER_SIZE)
#define TIMEOUT 1

#define WAITING_FOR_REQ 42
#define WAITING_FOR_ACK 43
#define MODE_SEND 50
#define MODE_RESEND 51

struct server_states {
	int state;
	int socketfd;
	struct sockaddr_in s_addr;
	struct sockaddr_in c_addr;
	socklen_t addr_len;
	char **packets;
	int *seqs;
	int *acks;
	int numpackets;
	int cwind;
	double cprob;
	double lprob;
	int last_ack;
	int last_sent;
	struct event_base *eb;
	struct event *bufferevt;
	struct event *timerevt;
};

void run_server(short portno, int cwind, double lost_rate, double corr_rate);
char *make_packet(int req, int fin, short len, int seq, const char data[DATA_SIZE]);
void read_packet(int *req, int *fin, short *len, int *seq, char data[DATA_SIZE], const char packet[PACKET_SIZE]);
struct timeval *get_timeout();
void bufferevt_handler(evutil_socket_t fd, short flag, void *arg);
void timerevt_handler(evutil_socket_t fd, short flag, void *arg);
void send_packets(struct server_states *ss, int mode);
int rngesus(float rate);

int main(int argc, char **argv) {
	srand(time(0));
	if (argc != 5) {
	bad_argument:
		fprintf(stderr, "Error: bad arguments.\n");
		exit(1);
	}
	short portno = (short) atoi(argv[1]);
	int cwind = atoi(argv[2]);
	double lost_rate = atof(argv[3]);
	double corr_rate = atof(argv[4]);

	if (portno < 0) goto bad_argument;
	if (cwind < 1) goto bad_argument;
	if (lost_rate < 0 || lost_rate > 1) goto bad_argument;
	if (corr_rate < 0 || corr_rate > 1) goto bad_argument;
	run_server(portno, cwind, lost_rate, corr_rate);
	return 0;
}

void run_server(short portno, int cwind, double lost_rate, double corr_rate) {
	// Bind the port indicated.
	struct server_states *ss = malloc(sizeof(struct server_states));
	int socket_fd;
	struct sockaddr_in server_addr, client_addr;
	char recv_buf[PACKET_SIZE], send_buf[PACKET_SIZE];
	socklen_t addr_len = sizeof(client_addr);

	bzero(&server_addr.sin_zero, sizeof(server_addr.sin_zero));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(portno);

	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		fprintf(stderr, "Error: socket.\n");
		exit(1);
	}

	if (bind(socket_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		fprintf(stderr, "Error: bind.\n");
		exit(1);
	}

	ss->socketfd = socket_fd;
	ss->s_addr = server_addr;
	ss->c_addr = client_addr;
	ss->addr_len = addr_len;
	ss->packets = NULL;
	ss->seqs = NULL;
	ss->acks = NULL;
	ss->numpackets = 0;
	ss->cwind = cwind;
	ss->cprob = corr_rate;
	ss->lprob = lost_rate;
	ss->last_ack = -1;
	ss->last_sent = -1;
	ss->eb = event_base_new();
	ss->timerevt = NULL;
	ss->state = WAITING_FOR_REQ;
	ss->bufferevt = event_new(ss->eb, ss->socketfd, EV_READ|EV_PERSIST, bufferevt_handler, ss);
	event_add(ss->bufferevt, NULL);
	event_base_dispatch(ss->eb);
	
	return;
}

char *make_packet(int req, int fin, short len, int seq, const char *data) {
	char *pkt = malloc(PACKET_SIZE);
	bzero(pkt, PACKET_SIZE);
	pkt[0] = (char) req;
	pkt[1] = (char) fin;
	*((short *) (pkt + 2)) = htons(len);
	*((int *) (pkt + 4)) = htonl(seq);
	if (data) {
		memcpy(pkt + 8, data, DATA_SIZE);
	}
	return pkt;
}

void read_packet(int *req, int *fin, short *len, int *seq, char data[DATA_SIZE], const char packet[PACKET_SIZE]) {
	int _req = (int) packet[0];
	int _fin = (int) packet[1];
	short _len = ntohs(*(short *)(packet + 2));
	int _seq = ntohl(*(int *)(packet + 4));
	memcpy(data, packet + 8, DATA_SIZE);
	*req = _req;
	*fin = _fin;
	*len = _len;
	*seq = _seq;

	return;
}

struct timeval *get_timeout() {
	struct timeval *ret = malloc(sizeof(struct timeval));
	ret->tv_sec = TIMEOUT;
	ret->tv_usec = 0;
	return ret;
}

void bufferevt_handler(evutil_socket_t fd, short flag, void *arg) {
	struct server_states *ss = (struct server_states *) arg;

	if (ss->state == WAITING_FOR_REQ) {
		char fname[DATA_SIZE];
		char pkt[PACKET_SIZE];
		recvfrom(ss->socketfd, pkt, PACKET_SIZE, 0, (struct sockaddr *) &ss->c_addr, &ss->addr_len);
		int req, fin, seq;
		short len;
		read_packet(&req, &fin, &len, &seq, fname, pkt);
		if (!req) {
			fprintf(stderr, "%s\n", "Error: not a request");
			return;
		}
		FILE *file = fopen(fname, "r");
		if (!file) {
			fprintf(stderr, "%s\n", "Error: file does not exist.");
			return;
		}
		struct stat metadata;
		bzero(&metadata, sizeof(struct stat));
		stat(fname, &metadata);
		off_t size = metadata.st_size;
		ss->numpackets = (size % DATA_SIZE == 0) ? (size / DATA_SIZE) : (size / DATA_SIZE + 1);
		ss->packets = malloc(ss->numpackets * sizeof(char *));
		ss->seqs = malloc(ss->numpackets * sizeof(int));
		ss->acks = malloc(ss->numpackets * sizeof(int));

		size_t bytes_read = 0;
		for (int i = 0; i < ss->numpackets; i++) {
			int firstbyte = bytes_read;
			char buf[DATA_SIZE];
			bzero(buf, DATA_SIZE);
			bytes_read += fread(buf, 1, DATA_SIZE, file);
			ss->seqs[i] = firstbyte;
			ss->acks[i] = bytes_read;

			if (i == ss->numpackets - 1) {
				ss->packets[i] = make_packet(0, 1, bytes_read - firstbyte, firstbyte, buf);
			} else {
				ss->packets[i] = make_packet(0, 0, bytes_read - firstbyte, firstbyte, buf);
			}
		}

		send_packets(ss, MODE_SEND);
		ss->timerevt = event_new(ss->eb, -1, EV_PERSIST, timerevt_handler, ss);
		evtimer_add(ss->timerevt, get_timeout());
		ss->state = WAITING_FOR_ACK;

	} else if (ss->state == WAITING_FOR_ACK) {
		char data[DATA_SIZE];
		char pkt[PACKET_SIZE];
		recvfrom(ss->socketfd, pkt, PACKET_SIZE, 0, (struct sockaddr *) &ss->c_addr, &ss->addr_len);
		int req, fin, ack;
		short len;
		read_packet(&req, &fin, &len, &ack, data, pkt);
		if (req) {
			fprintf(stderr, "%s\n", "Error: not an ack");
			return;
		}


		for (int i = 0; i < ss->numpackets; i++) {
			
			if (ss->acks[i] == ack && i > ss->last_ack) {
				printf("Expecting ACK: %d, Received ACK: %d\n", ss->acks[i], ack);
				if (i != ss->numpackets - 1) {
					ss->last_ack = i;
					evtimer_del(ss->timerevt);
					send_packets(ss, MODE_SEND);
					evtimer_add(ss->timerevt, get_timeout());
				} else {
					ss->last_ack = i;
					printf("Task complete, shut down.\n");
					exit(0); // Receiver has acked the last packet.
				}
			}
		}

	}

	return;
}

void timerevt_handler(evutil_socket_t fd, short flag, void *arg) {
	struct server_states *ss = (struct server_states *) arg;
	send_packets(ss, MODE_RESEND);

	return;
}

void send_packets(struct server_states *ss, int mode) {

	int ub = (ss->last_ack + ss->cwind < ss->numpackets) ? (ss->last_ack + ss->cwind + 1) : (ss->numpackets);
	if (mode == MODE_SEND) {
		for (int i = ss->last_sent + 1; i < ub; i++) {
			if (rngesus(ss->lprob)) {
				printf("Sending packet %d of %d, but it is lost.\n", i + 1, ss->numpackets);
			} else {
				sendto(ss->socketfd, ss->packets[i], PACKET_SIZE, 0, (struct sockaddr *) &ss->c_addr, ss->addr_len);
				printf("Sending packet %d of %d.\n", i + 1, ss->numpackets);
			}
			ss->last_sent++;
		}
	} else if (mode == MODE_RESEND) {
		for (int i = ss->last_ack + 1; i < ub; i++) {
			if (rngesus(ss->lprob)) {
				printf("Resending packet %d of %d, but it is lost.\n", i + 1, ss->numpackets);
			} else {
				sendto(ss->socketfd, ss->packets[i], PACKET_SIZE, 0, (struct sockaddr *) &ss->c_addr, ss->addr_len);
				printf("Resending packet %d of %d.\n", i + 1, ss->numpackets);
			}
		}
	}
	return;
}

int rngesus(float rate) {
	int hitnum = (int) (rate * 100);
	int result = rand() % 100;
	if (result < hitnum) {
		return 1;
	} else {
		return 0;
	}
}	