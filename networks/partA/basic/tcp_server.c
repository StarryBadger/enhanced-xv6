#include "headers.h"
int main()
{
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        close(server_sock);
        exit(1);
    }
    printf("[+]Bind to the port number: %d\n", port);

    if (listen(server_sock, 5) < 0)
    {
        fprintf(stderr, "[-]Listen error: %s\n", strerror(errno));
        close(server_sock);
        exit(1);
    }
    printf("Listening...\n");
    int flag=0;
    while (1)
    {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        if (client_sock < 0)
        {
            fprintf(stderr, "[-]Accept error: %s\n", strerror(errno));
            if (close(server_sock) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+]Client connected.\n");

        bzero(buffer, 1024);
        if (recv(client_sock, buffer, sizeof(buffer), 0) < 0)
        {
            fprintf(stderr, "[-]Receive error: %s\n", strerror(errno));
            if (close(client_sock) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            if (close(server_sock) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }
        printf("Client sent a message: %s\n", buffer);
        printf("Say something back: ");
        buffer[0] = '\0';
        if (flag)
        {
            getchar();
        }
        if (scanf("%1023[^\n]", buffer) != 1)
        {
            fprintf(stderr, "[-]Error reading input");
            exit(1);
        }
        printf("Message being sent back to client: %s\n", buffer);
        if (send(client_sock, buffer, strlen(buffer), 0) < 0)
        {
            fprintf(stderr, "[-]Send error: %s\n", strerror(errno));
            if (close(client_sock) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            if (close(server_sock) < 0)
                fprintf(stderr, "[-]Error closing socket: %s\n", strerror(errno));
            exit(1);
        }

        if (close(client_sock) < 0)
        {
            fprintf(stderr, "[-]Error closing client socket: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+]Client disconnected.\n\n");
        flag=1;
    }

    if (close(server_sock) < 0)
    {
        fprintf(stderr, "[-]Error closing server socket: %s\n", strerror(errno));
        exit(1);
    }
    return 0;
}