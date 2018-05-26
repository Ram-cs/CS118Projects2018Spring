

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


#define MAX_SEQ_NUM 30720
#define MAX_PKT_LENGTH 1024
#define TIMEOUT 500
#define WND_SIZE 5120


#define SYN 1
#define STATUS 2
#define DATA 3
#define ACK 4
#define FIN 5



#endif /* support_h */
