#ifndef __HEADERS_H_
#define __HEADERS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <poll.h>
#include<string.h>


#define CHUNK_SIZE 4
#define RETRANSMISSION_TIME_USEC 100000
#define ip "127.0.0.1"
#define port 5566

typedef struct data
{
    int sequenceNo;
    char data[CHUNK_SIZE+1];
} data;

#endif