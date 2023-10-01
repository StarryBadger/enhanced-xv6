#include "headers.h"
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
int main()
{
    int server_sockA, server_sockB, client_sockA, client_sockB;
    struct sockaddr_in server_addrA, server_addrB, client_addrA, client_addrB;
    socklen_t addr_sizeA, addr_sizeB;
    char A[128];
    char B[128];
    int a, b;
    int scoreA = 0, scoreB = 0;
    int n;

    server_sockA = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockA < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }
    server_sockB = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockB < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]TCP server sockets created.\n");
    memset(&server_addrA, '\0', sizeof(server_addrA));
    server_addrA.sin_family = AF_INET;
    server_addrA.sin_port = portA;
    server_addrA.sin_addr.s_addr = inet_addr(ip);

    memset(&server_addrB, '\0', sizeof(server_addrB));
    server_addrB.sin_family = AF_INET;
    server_addrB.sin_port = portB;
    server_addrB.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_sockA, (struct sockaddr *)&server_addrA, sizeof(server_addrA));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        close(server_sockA);
        exit(1);
    }
    n = bind(server_sockB, (struct sockaddr *)&server_addrB, sizeof(server_addrB));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        close(server_sockB);
        exit(1);
    }
    printf("[+]Binded to the port numbers: %d and %d\n", portA, portB);
    if (listen(server_sockA, 5) < 0)
    {
        fprintf(stderr, "[-]Listen error: %s\n", strerror(errno));
        close(server_sockA);
        exit(1);
    }
    if (listen(server_sockB, 5) < 0)
    {
        fprintf(stderr, "[-]Listen error: %s\n", strerror(errno));
        close(server_sockB);
        exit(1);
    }
    printf("Listening...\n");
    addr_sizeA = sizeof(client_addrA);
    client_sockA = accept(server_sockA, (struct sockaddr *)&client_addrA, &addr_sizeA);
    if (client_sockA < 0)
    {
        fprintf(stderr, "[-]Accept error: %s\n", strerror(errno));
        if (close(server_sockA) < 0)
            fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]Player A connected.\n");
    addr_sizeB = sizeof(client_addrB);
    client_sockB = accept(server_sockB, (struct sockaddr *)&client_addrB, &addr_sizeB);
    if (client_sockB < 0)
    {
        fprintf(stderr, "[-]Accept error: %s\n", strerror(errno));
        if (close(server_sockB) < 0)
            fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]Player B connected.\n");
    while (1)
    {
        bzero(A, 128);
        receiver(client_sockA, client_sockB, A, server_sockA, server_sockB);
        a = myatoi(A);
        bzero(A, 128);

        bzero(B, 128);
        receiver(client_sockB, client_sockA, B, server_sockB, server_sockA);
        b = myatoi(B);
        bzero(B, 128);

        statOpponent(b, A);
        statOpponent(a, B);

        if ((a == 0 && b == 1) || (a == 1 && b == 2) || (a == 2 && b == 0))
        {
            strcat(A, "You lose!");
            strcat(B, "You win!");
            printf("Player B won this round!\n");
            scoreB++;
        }
        else if (a == b)
        {
            strcat(A, "It's a draw!");
            strcat(B, "It's a draw!");
            printf("It's a draw between the players!\n");
        }
        else if ((b == 0 && a == 1) || (b == 1 && a == 2) || (b == 2 && a == 0))
        {
            strcat(A, "You win!");
            strcat(B, "You lose!");
            printf("Player A won this round!\n");
            scoreA++;
        }
        else
        {
            strcat(A, "Some error occured in sending your choice!");
            strcat(B, "Some error occured in sending your choice!");
        }
        sender(client_sockA, client_sockB, A, server_sockA, server_sockB);
        sender(client_sockB, client_sockA, B, server_sockB, server_sockA);

        bzero(A, 128);
        bzero(B, 128);
        receiver(client_sockA, client_sockB, A, server_sockA, server_sockB);
        receiver(client_sockB, client_sockA, B, server_sockB, server_sockA);
        a = myatoi(A);
        bzero(A, 128);
        b = myatoi(B);
        bzero(B, 128);
        if (a==0||b==0)
        {
            bzero(A, 128);
            bzero(B, 128);
            sprintf(A, "No rematch. Game ended! You won %d games and your opponent won %d games.", scoreA, scoreB);
            sprintf(B, "No rematch. Game ended! You won %d games and your opponent won %d games.", scoreB, scoreA);
            sender(client_sockA, client_sockB, A, server_sockA, server_sockB);
            sender(client_sockB, client_sockA, B, server_sockB, server_sockA);
            break;
        }
        else
        {
            bzero(A, 128);
            bzero(B, 128);
            strcpy(A, "Rematch accepted!");
            strcpy(B, "Rematch accepted!");
            sender(client_sockA, client_sockB, A, server_sockA, server_sockB);
            sender(client_sockB, client_sockA, B, server_sockB, server_sockA);
        }
    }
    printf("Game ended! Player A won %d games and player B won %d games.\n", scoreA, scoreB);
    closer(client_sockA, client_sockB, server_sockA, server_sockB);
    return 0;
}