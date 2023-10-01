#ifndef __HEADERS_H_
#define __HEADERS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define ip "127.0.0.1"
#define portA 5566
#define portB 5568

int myatoi(char *str);
void closer(int client_sock1, int client_sock2, int server_sock1, int server_sock2);
void receiver(int client_sock1, int client_sock2, char *buffer, int server_sock1, int server_sock2);
void sender(int client_sock1, int client_sock2, char *buffer, int server_sock1, int server_sock2);
void statOpponent(int a, char *buffer);
void statYou(int a, char *buffer);
#endif