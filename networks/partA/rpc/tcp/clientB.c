#include "headers.h"
int main()
{
    int mySocket;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[128];
    buffer[0] = '\0';
    int n, x;
    int flag = 0;

    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]TCP socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = portB;
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(mySocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        fprintf(stderr, "[-]Connection error: %s\n", strerror(errno));
        exit(1);
    }
    printf("Welcome, player B!\n");
    while (1)
    {
        printf("Your move! Enter 0 for rock, 1 for paper and 2 for scissors\n");
        if (flag)
        {
            getchar();
        }
        if (scanf("%s", buffer) != 1)
        {
            fprintf(stderr, "[-]Error reading input");
            exit(1);
        }
        getchar();
        int x = myatoi(buffer);
        if (x == -1)
        {
            printf("Invalid choice! Re-enter\n");
            continue;
        }
        if (send(mySocket, buffer, strlen(buffer), 0) < 0)
        {
            fprintf(stderr, "[-]Send error: %s\n", strerror(errno));
            if (close(mySocket) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        bzero(buffer, 128);
        statYou(x, buffer);
        printf("%s\n", buffer);
        bzero(buffer, 128);
        if (recv(mySocket, buffer, sizeof(buffer), 0) < 0)
        {
            fprintf(stderr, "[-]Receive error: %s\n", strerror(errno));
            if (close(mySocket) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        printf("%s\n", buffer);
        bzero(buffer, 128);
        printf("Press the R key to request a rematch. Press any other key to end game. Press Enter to confirm   your choice\n");
        char c = getchar();
        if (c == 'r' || c == 'R')
        {
            strcpy(buffer, "1");
        }
        else
        {
            strcpy(buffer, "0");
        }
        if (send(mySocket, buffer, strlen(buffer), 0) < 0)
        {
            fprintf(stderr, "[-]Send error: %s\n", strerror(errno));
            if (close(mySocket) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        bzero(buffer, 128);
        if (recv(mySocket, buffer, sizeof(buffer), 0) < 0)
        {
            fprintf(stderr, "[-]Receive error: %s\n", strerror(errno));
            if (close(mySocket) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        printf("%s\n",buffer);
        if (buffer[0] == 'N')
        {
            break;
        }
        flag = 1;
    }
    if (close(mySocket) < 0)
    {
        fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
        exit(1);
    }
    return 0;
}