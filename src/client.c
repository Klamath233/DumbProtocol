#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "../include/event2/event.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define PACKET_SIZE 1024
#define HEADER_SIZE 8
#define DATA_SIZE PACKET_SIZE - HEADER_SIZE
#define REG_TIMEOUT 1
#define FIN_TIMEOUT 10

#define WAITING_FOR_DATA 44
#define FINAL_WAITING 45
/*

REQ: 1. request file
     0. ACK

FIN
LEN
SEQ

*/

/* 

Client (receiver): 
./client server_hostname 
         server_port_number
         filename
         prob_loss
         prob_corruption
*/
#define DEBUG 0
#define LOG

struct client_states {
  int state;
  int socketfd;
  struct sockaddr_in s_addr;
  socklen_t addr_len;
  FILE *file;
  double cprob;
  double lprob;
  int seq_expected;
  struct event_base *eb;
  struct event *bufferevt;
  struct event *timerevt;
  char *request;
};

void run_client(char* hostname, short portno, 
                char* fn, float lost_rate, float corr_rate);

void make_ACK(char* pkt, int seq);
void make_file_request(char* pkt, int seq, char* filename);
int readReq(char* pkt)
{
  return (int)(pkt[0]);
}

int readFin(char* pkt)
{
  return (int)(pkt[1]);
}

int readLen(char* pkt)
{
  return ntohs(*(short*)(pkt+2));
}

int readSeq(char* pkt)
{
  return ntohl(*(int*)(pkt+4));
}

struct timeval *get_timeout(int t);
void read_data(char data[DATA_SIZE], const char packet[PACKET_SIZE]);
void bufferevt_handler(evutil_socket_t fd, short flag, void *arg);
void timerevt_handler(evutil_socket_t fd, short flag, void *arg);
int rngesus(float rate);
// -------------------------------



int main(int argc, char **argv)
{
  if(argc != 6){
  bad_argument:
    fprintf(stderr, "Error: bad arguments.\n");
    exit(1);
  }
  char* server_hostname = argv[1];

  short portno = (short)atoi(argv[2]);
  char* filename = argv[3];

  float lost_rate = atof(argv[4]);
  float corr_rate = atof(argv[5]);

  srand(time(0));
  if(portno < 0) goto bad_argument;
  if(lost_rate < 0 || lost_rate > 1) goto bad_argument;
  if(corr_rate < 0 || corr_rate > 1) goto bad_argument;

  run_client(server_hostname, portno, filename, lost_rate, corr_rate);
  return 0;

}


void run_client(char* hostname, short portno, char* fn, float lost_rate, float corr_rate)
{
  if(DEBUG)
  {
    printf("host name: %s\n", hostname);
    printf("port no: %hi\n", portno);
    printf("file name: %s\n", fn);
    printf("lost rate: %f\n", lost_rate);
    printf("corr rate: %f\n", corr_rate);
  }

  #ifdef LOG
  printf("Client running.\n");
  printf("Server: %s:%hi.\n", hostname, portno);
  printf("File: %s\n", fn);
  printf("Client side corruption rate: %f\n", corr_rate);
  printf("Client side lost rate: %f\n", lost_rate);
  #endif

  //inet_aton
  struct client_states *cs = malloc(sizeof(struct client_states));
  struct sockaddr_in server_addr;
  int sockfd;
  char recv_buf[PACKET_SIZE];
  char *send_buf = malloc(PACKET_SIZE);
  char file_rev[DATA_SIZE];
  socklen_t addr_len;
  int n;

  /* Initialization */
  bzero(&server_addr, sizeof(server_addr));
  bzero(recv_buf, PACKET_SIZE);
  bzero(send_buf, PACKET_SIZE);
  bzero(file_rev, DATA_SIZE);

  if( 0 == inet_aton(hostname, &server_addr.sin_addr))
  {
    fprintf(stderr,"Hostname is invalid\nExitting...\n");
    exit(1);
  }
  server_addr.sin_port = htons(portno);
  server_addr.sin_family = AF_INET;
  addr_len = sizeof(server_addr);

  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    fprintf(stderr, "Cannot create socket\nExitting\n");
    exit(1);
  }

  // First, make file request
  make_file_request(send_buf, 0, fn);

  FILE *file = fopen(fn, "w");
  if (!file) {
    fprintf(stderr,"Cannnot open file\nExitting...\n");
    exit(1);
  }

  #ifdef LOG
  printf("File created.\n");
  #endif

  fseek(file, 0L, SEEK_SET);
  if (rngesus(lost_rate)) {
    printf("Client request will lost!\n");
  } else {
    n = sendto(sockfd, send_buf, PACKET_SIZE, 0, (struct sockaddr *) &server_addr, addr_len);
      #ifdef LOG
      printf("Request sent.\n");
      #endif
  }

  cs->state = WAITING_FOR_DATA;
  cs->socketfd = sockfd;
  cs->s_addr = server_addr;
  cs->addr_len = addr_len;
  cs->file = file;
  cs->cprob = corr_rate;
  cs->lprob = lost_rate;
  cs->seq_expected = 0;
  cs->eb = event_base_new();
  cs->timerevt = event_new(cs->eb, -1, EV_PERSIST, timerevt_handler, cs);
  cs->bufferevt = event_new(cs->eb, cs->socketfd, EV_READ|EV_PERSIST, bufferevt_handler, cs);
  cs->request = send_buf;
  evtimer_add(cs->timerevt, get_timeout(REG_TIMEOUT));
  event_add(cs->bufferevt, NULL);
  event_base_dispatch(cs->eb);

  return;
}


void make_ACK(char* pkt, int seq)
{
  int i = 0;
  bzero(pkt, PACKET_SIZE);
  short zero = 0;
  pkt[0] = (char)zero;
  pkt[1] = (char)zero;
  *((short*)(pkt + 2)) = htons(zero);
  *((int*)(pkt+4)) = htonl(seq);
  if(DEBUG)
  {
    for(i=0;i<4;i++)
    {
      printf("%d: %c\n", i, pkt[i]);
    }
    printf("%d\n",ntohl(*(int *)(pkt + 4)));
  }
}


// Size of filename should not be larger than 1016
void make_file_request(char* pkt, int seq, char* filename)
{
  //char* pkt = (char*)malloc(PACKET_SIZE);
  bzero(pkt, PACKET_SIZE);
  short zero = 0, one = 1;
  *pkt = (char)one;
  *(pkt+1) = (char)zero;
  char* a = filename;
  short length = 0;
  while(*(a++))
    length++;
  if(DEBUG) printf("filename length: %d\n", length);
  if(length > 1016)
  {
    fprintf(stderr, "File name too long! Exitting...\n" );
    exit(1);
  }
  *((short*)(pkt+2)) = htons(length);
  *((int*)(pkt+4)) = htonl(seq);
  strcpy(pkt + 8, filename);
}

void bufferevt_handler(evutil_socket_t fd, short flag, void *arg) {
  struct client_states *cs = (struct client_states *) arg;

  if (cs->state == WAITING_FOR_DATA) {
    char sendbuf[PACKET_SIZE];
    char recvbuf[PACKET_SIZE];
    char databuf[DATA_SIZE];
    bzero(recvbuf, PACKET_SIZE);
    bzero(sendbuf, PACKET_SIZE);
    bzero(databuf, DATA_SIZE);

    recvfrom(cs->socketfd, recvbuf, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, &cs->addr_len);
    if (rngesus(cs->cprob)) {
      printf("Received Corrupted Packet.\n");
      return;
    }

    if (readSeq(recvbuf) > cs->seq_expected) {
      // Out of order
      #ifdef LOG
      printf("Expecting SEQ: %d. Reiceived SEQ: %d. Out of order.\n", cs->seq_expected, readSeq(recvbuf));
      #endif
      evtimer_del(cs->timerevt);
      event_free(cs->timerevt);
      make_ACK(sendbuf, cs->seq_expected);
      if (rngesus(cs->lprob)) {
        printf("Reacking for SEQ: %d, but it will lost.\n", cs->seq_expected);
      } else { 
        printf("Reacking for SEQ: %d.\n", cs->seq_expected);
        sendto(cs->socketfd, sendbuf, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, cs->addr_len);
      }
      cs->timerevt = event_new(cs->eb, -1, EV_PERSIST, timerevt_handler, cs);
      evtimer_add(cs->timerevt, get_timeout(REG_TIMEOUT));
      return;
    } else if (readSeq(recvbuf) < cs->seq_expected) {
      #ifdef LOG
      printf("Expecting SEQ: %d. Reiceived SEQ: %d. Repeated.\n", cs->seq_expected, readSeq(recvbuf));
      #endif
      return;
    }

    #ifdef LOG
    printf("Expecting SEQ: %d. Reiceived SEQ: %d. OK.\n", cs->seq_expected, readSeq(recvbuf));
    #endif
    evtimer_del(cs->timerevt);
    read_data(databuf, recvbuf);

    int len = readLen(recvbuf);
    fwrite(databuf, 1, (size_t) len, cs->file);
    cs->seq_expected += len;

    make_ACK(sendbuf, cs->seq_expected);
    if (rngesus(cs->lprob)) {
        printf("Acking for SEQ: %d, but it will lost.\n", cs->seq_expected);
      } else { 
        printf("Acking for SEQ: %d.\n", cs->seq_expected);
        sendto(cs->socketfd, sendbuf, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, cs->addr_len);
      }

    if (readFin(recvbuf) == 1) {
      sendbuf[1] = 1;
      if (rngesus(cs->lprob)) {
        printf("Sending FIN to server, but it will lost.\n");
      } else { 
        printf("Sending FIN to server.\n");
        sendto(cs->socketfd, sendbuf, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, cs->addr_len);
      }
      fclose(cs->file);
      cs->state = FINAL_WAITING;
    }

    evtimer_add(cs->timerevt, get_timeout(REG_TIMEOUT));
  } else if (cs->state == FINAL_WAITING) {
    char sendbuf[PACKET_SIZE];
    char recvbuf[PACKET_SIZE];
    char databuf[DATA_SIZE];
    bzero(recvbuf, PACKET_SIZE);
    bzero(sendbuf, PACKET_SIZE);
    bzero(databuf, DATA_SIZE);

    recvfrom(cs->socketfd, recvbuf, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, &cs->addr_len);
    if (rngesus(cs->cprob)) {
      printf("Received Corrupted Packet.\n");
      return;
    }
    if (readReq(recvbuf) == 1) {
      exit(0);
    }
  }
}

void timerevt_handler(evutil_socket_t fd, short flag, void *arg) {
  struct client_states *cs = (struct client_states *) arg;
  char sendbuf[PACKET_SIZE];
  #ifdef LOG
  printf("Expecting SEQ: %d. Timeout\n", cs->seq_expected);
  #endif
  if (cs->seq_expected == 0) {
    // Resend the request and return.
    if (rngesus(cs->lprob)) {
        printf("Resending the request to server, but it will lost.\n");
      } else { 
        printf("Resending the request to server.\n");
        sendto(cs->socketfd, cs->request, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, cs->addr_len);
      }
      return;
  }
  make_ACK(sendbuf, cs->seq_expected);
  if (rngesus(cs->lprob)) {
        printf("Reacking for SEQ: %d, but it will lost.\n", cs->seq_expected);
      } else { 
        printf("Reacking for SEQ: %d.\n", cs->seq_expected);
        sendto(cs->socketfd, sendbuf, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, cs->addr_len);
      }
  if (cs->state == FINAL_WAITING) {
    sendbuf[1] = 1;
    if (rngesus(cs->lprob)) {
        printf("Resending FIN to server, but it will lost.\n");
      } else { 
        printf("Resending FIN to server\n");
        sendto(cs->socketfd, sendbuf, PACKET_SIZE, 0, (struct sockaddr *) &cs->s_addr, cs->addr_len);
      }
  }
}

void read_data(char data[DATA_SIZE], const char packet[PACKET_SIZE]) {
  memcpy(data, packet + 8, DATA_SIZE);
  return;
}

struct timeval *get_timeout(int t) {
  struct timeval *ret = malloc(sizeof(struct timeval));
  ret->tv_sec = t;
  ret->tv_usec = 0;
  return ret;
}

int rngesus(float rate) {
  int hitnum = (int) (rate * 100);
  int result = rand() % 100;
  // printf("hitnum: %d, result %d.\n", hitnum, result);
  if (result < hitnum) {
    return 1;
  } else {
    return 0;
  }
}
