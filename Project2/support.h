

#ifndef support_h
#define support_h
//CLIENT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//SERVER
#include <string.h>
#include <arpa/inet.h>

#include <sys/timeb.h>  // timestamp in millisecond
#include <signal.h>


#define MAX_SEQ_NUM 30720
#define MAX_PKT_LENGTH 1024
#define HEADER_SIZE 20
#define PAYLOAD_SIZE 1004
#define TIMEOUT 500
#define WND_SIZE 5120
#define NUM_PKG 10 //number of package allowed to send



#define IS_SEQNUM 1
#define IS_SYN 2
#define IS_ACK 3
#define IS_FIN 4
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct
{
    int seqNum;
    int ackNum;
    int SYN;
    int FIN;
    int PKG_TYPE;
    char payload[MAX_PKT_LENGTH];
} TCP_Packet;


//PRINT ERROR
void error(const char* error_message) {
    perror(error_message);
    exit(1);
}

void send_packet(int sockfd, struct sockaddr_in addr, TCP_Packet packet) { //sending packages
    printf("Sending packet%d", packet.PKG_TYPE ? packet.ackNum : packet.seqNum); //if it is ACK or sequence number
    
    if(sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
        error("ERROR, can not send package");
    }
}


/*
 #define MAX_SEQ_NUM 30720;
 #define MAX_PKT_LENGTH 1024;
 #define TIMEOUT 500;
 #define WND_SIZE 5120;
 */

//#define SYN 1
//#define STATUS 2
//#define DATA 3
//#define ACK 4
//#define FIN 5




//FOR CONGESTION WINDOW
int ssthreash = 15360;
int cwnd = 1024;
int dup_ACK_COUNT = 0;



/*
 //GET SYSTEM TIMESTAMP IN MILLISECONDS
 //ref:https://programmingtoddler.wordpress.com/2014/11/09/c-how-to-get-system-timestamp-in-second-millisecond-and-microsecond/
 long long sys_timestamp() {
 struct timeb timer_msec;
 long long int timestamp_msec;
 if (!ftime(&timer_msec)) {
 timestamp_msec = ((long long int) timer_msec.time) * 1000ll +
 (long long int) timer_msec.millitm;
 }
 else {
 timestamp_msec = -1;
 }
 return timestamp_msec;
 }
 
 
 //set congestion window
 void set_cwnd(int value) {
 cwnd = MIN(value, MAX_SEQ_NUM/2);
 }
 */

#endif /* support_h */
