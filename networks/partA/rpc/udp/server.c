#include "headers.h"
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
void identify(int a)
{
    if (a == 0)
    {
        printf("rock.\n");
    }
    else if (a == 1)
    {
        printf("paper.\n");
    }
    else
    {
        printf("scissors.\n");
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
    int mySocketA, mySocketB;
    struct sockaddr_in server_addrA, server_addrB, client_addrA, client_addrB;
    socklen_t addr_sizeA, addr_sizeB;
    char A[128];
    char B[128];
    int a, b;
    int scoreA = 0, scoreB = 0;
    int n;

    mySocketA = socket(AF_INET, SOCK_DGRAM, 0);
    if (mySocketA < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }
    mySocketB = socket(AF_INET, SOCK_DGRAM, 0);
    if (mySocketB < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]UDP server sockets created.\n");
    memset(&server_addrA, '\0', sizeof(server_addrA));
    server_addrA.sin_family = AF_INET;
    server_addrA.sin_port = htons(portA);
    server_addrA.sin_addr.s_addr = inet_addr(ip);

    memset(&server_addrB, '\0', sizeof(server_addrB));
    server_addrB.sin_family = AF_INET;
    server_addrB.sin_port = htons(portB);
    server_addrB.sin_addr.s_addr = inet_addr(ip);

    n = bind(mySocketA, (struct sockaddr *)&server_addrA, sizeof(server_addrA));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        exit(1);
    }
    n = bind(mySocketB, (struct sockaddr *)&server_addrB, sizeof(server_addrB));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]Binded to the port numbers: %d and %d\n", portA, portB);
    addr_sizeA = sizeof(client_addrA);
    addr_sizeB = sizeof(client_addrB);
    while (1)
    {
        bzero(A, 128);
        if (recvfrom(mySocketA, A, 128, 0, (struct sockaddr *)&client_addrA, &addr_sizeA) < 0)
        {
            fprintf(stderr, "[-]Error while receiving data\n");
            exit(1);
        }
        a = myatoi(A);
        bzero(A, 128);

        bzero(B, 128);
        if (recvfrom(mySocketB, B, 128, 0, (struct sockaddr *)&client_addrB, &addr_sizeB) < 0)
        {
            fprintf(stderr, "[-]Error while receiving data\n");
            exit(1);
        }
        b = myatoi(B);
        bzero(B, 128);
        statOpponent(b, A);
        statOpponent(a, B);

        if ((a == 0 && b == 1) || (a == 1 && b == 2) || (a == 2 && b == 0))
        {
            strcat(A, "You lose!");
            strcat(B, "You win!");
            printf("Player A played ");
            identify(a);
            printf("Player B played ");
            identify(b);
            printf("Player B won this round!\n");
            scoreB++;
        }
        else if (a == b)
        {
            strcat(A, "It's a draw!");
            strcat(B, "It's a draw!");
            printf("Player A played ");
            identify(a);
            printf("Player B played ");
            identify(b);
            printf("It's a draw between the players!\n");
        }
        else if ((b == 0 && a == 1) || (b == 1 && a == 2) || (b == 2 && a == 0))
        {
            strcat(A, "You win!");
            strcat(B, "You lose!");
            printf("Player A played ");
            identify(a);
            printf("Player B played ");
            identify(b);
            printf("Player A won this round!\n");
            scoreA++;
        }
        else
        {
            strcat(A, "Some error occured in sending your choice!");
            strcat(B, "Some error occured in sending your choice!");
        }
        if (sendto(mySocketA, A, 128, 0, (struct sockaddr *)&client_addrA, sizeof(client_addrA)) < 0)
        {
            fprintf(stderr, "[-]Error while receiving data\n");
            exit(1);
        }
        if (sendto(mySocketB, B, 128, 0, (struct sockaddr *)&client_addrB, sizeof(client_addrB)) < 0)
        {
            fprintf(stderr, "[-]Error while receiving data\n");
            exit(1);
        }

        bzero(A, 128);
        bzero(B, 128);
        if (recvfrom(mySocketA, A, 128, 0, (struct sockaddr *)&client_addrA, &addr_sizeA) < 0)
        {
            fprintf(stderr, "[-]Error while receiving data\n");
            exit(1);
        }
        if (recvfrom(mySocketB, B, 128, 0, (struct sockaddr *)&client_addrB, &addr_sizeB) < 0)
        {
            fprintf(stderr, "[-]Error while receiving data\n");
            exit(1);
        }
        a = myatoi(A);
        bzero(A, 128);
        b = myatoi(B);
        bzero(B, 128);
        if (a == 0 || b == 0)
        {
            bzero(A, 128);
            bzero(B, 128);
            sprintf(A, "No rematch. Game ended! You won %d games and your opponent won %d games.", scoreA, scoreB);
            sprintf(B, "No rematch. Game ended! You won %d games and your opponent won %d games.", scoreB, scoreA);
            if (sendto(mySocketA, A, 128, 0, (struct sockaddr *)&client_addrA, sizeof(client_addrA)) < 0)
            {
                fprintf(stderr, "[-]Error while receiving data\n");
                exit(1);
            }
            if (sendto(mySocketB, B, 128, 0, (struct sockaddr *)&client_addrB, sizeof(client_addrB)) < 0)
            {
                fprintf(stderr, "[-]Error while receiving data\n");
                exit(1);
            }
            break;
        }
        else
        {
            bzero(A, 128);
            bzero(B, 128);
            strcpy(A, "Rematch accepted!");
            strcpy(B, "Rematch accepted!");
            if (sendto(mySocketA, A, 128, 0, (struct sockaddr *)&client_addrA, sizeof(client_addrA)) < 0)
            {
                fprintf(stderr, "[-]Error while receiving data\n");
                exit(1);
            }
            if (sendto(mySocketB, B, 128, 0, (struct sockaddr *)&client_addrB, sizeof(client_addrB)) < 0)
            {
                fprintf(stderr, "[-]Error while receiving data\n");
                exit(1);
            }
        }
    }
    printf("Game ended! Player A won %d games and player B won %d games.\n", scoreA, scoreB);
    return 0;
}