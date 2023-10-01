#include "headers.h"
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
            scoreB++;
        }
        else if (a == b)
        {
            strcat(A, "It's a draw!");
            strcat(B, "It's a draw!");
        }
        else if ((b == 0 && a == 1) || (b == 1 && a == 2) || (b == 2 && a == 0))
        {
            strcat(A, "You win!");
            strcat(B, "You lose!");
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
        // printf("a: %d b: %d\n",a,b);
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
    closer(client_sockA, client_sockB, server_sockA, server_sockB);
    return 0;
}