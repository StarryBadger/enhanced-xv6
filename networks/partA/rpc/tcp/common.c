#include "headers.h"
int myatoi(char *str)
{
    int n = 0;
    for (int i = 0; i < strlen(str); i++)
    {

        int num = (int)str[i] - 48;
        if (num >= 0 && num <= 9)
            n = n * 10 + num;
        else
            return -1;
    }
    if (n > 2)
    {
        return -1;
    }
    return n;
}
void closer(int client_sock1, int client_sock2, int server_sock1, int server_sock2)
{
    int flag = 0;
    if (close(client_sock1) < 0)
    {
        fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        flag = 1;
    }
    if (close(client_sock2) < 0)
    {
        fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        flag = 1;
    }
    if (close(server_sock1) < 0)
    {
        fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        flag = 1;
    }
    if (close(server_sock2) < 0)
    {
        fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        flag = 1;
    }
    if (flag)
    {
        exit(1);
    }
}
void receiver(int client_sock1, int client_sock2, char *buffer, int server_sock1, int server_sock2)
{
    if (recv(client_sock1, buffer, sizeof(buffer), 0) < 0)
    {
        fprintf(stderr, "[-]Receive error: %s\n", strerror(errno));
        closer(client_sock1, client_sock2, server_sock1, server_sock2);
        exit(1);
    }
}
void sender(int client_sock1, int client_sock2, char *buffer, int server_sock1, int server_sock2)
{
    if (send(client_sock1, buffer, strlen(buffer), 0) < 0)
    {
        fprintf(stderr, "[-]Send error: %s\n", strerror(errno));
        closer(client_sock1, client_sock2, server_sock1, server_sock2);
        exit(1);
    }
}
void statYou(int a, char *buffer)
{
    strcpy(buffer, "You played ");
    if (a == 0)
    {
        strcat(buffer, "rock.");
    }
    else if (a == 1)
    {
        strcat(buffer, "paper.");
    }
    else
    {
        strcat(buffer, "scissors.");
    }
}
void statOpponent(int a, char *buffer)
{
    strcpy(buffer, "Your oppenent played ");
    if (a == 0)
    {
        strcat(buffer, "rock.\n");
    }
    else if (a == 1)
    {
        strcat(buffer, "paper.\n");
    }
    else
    {
        strcat(buffer, "scissors.\n");
    }
}