#include "headers.h"
int main()
{
    int mySocket;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];
    socklen_t addr_size;
    int n;

    mySocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mySocket < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(mySocket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        fprintf(stderr, "[-]Bind error: %s\n", strerror(errno));
        exit(1);
    }
    int chunkCount;
    addr_size = sizeof(client_addr);
    if (recvfrom(mySocket, &chunkCount, sizeof(chunkCount), 0, (struct sockaddr *)&client_addr, &addr_size) < 0)
    {
        fprintf(stderr, "[-]Error while receiving chunkCount\n");
        exit(1);
    }
    printf("To receive: %d packets of data\n", chunkCount);

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 20;
    setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    data packets[chunkCount];
    int receivedRegister[chunkCount];
    for (int i = 0; i < chunkCount; i++)
    {
        receivedRegister[i] = 0;
    }
    int receivedCount = 0;
    // int flag[chunkCount];
    // for (int i = 0; i < chunkCount; i++)
    // {
    //     flag[i] = 0;
    // }
    while (receivedCount < chunkCount)
    {
        struct data packet;
        if (recvfrom(mySocket, &packet, sizeof(packet), 0, NULL, NULL) < 0 && (errno != EWOULDBLOCK))
        {
            fprintf(stderr, "[-]Error while receiving data chunk\n");
            exit(1);
        }
        printf("Sequence number: %d Data Packet: %s\n", packet.sequenceNo, packet.data);
        // if ((packet.sequenceNo % 2 == 0) && flag[packet.sequenceNo] < 10)
        // {
        //     flag[packet.sequenceNo] += 1;
        //     continue;
        // }
        if (!receivedRegister[packet.sequenceNo])
        {
            receivedRegister[packet.sequenceNo] = 1;
            packets[packet.sequenceNo] = packet;
            receivedCount++;
        }

        if (sendto(mySocket, &packet.sequenceNo, sizeof(packet.sequenceNo), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        {
            fprintf(stderr, "[-]Error while sending acknowledgement\n");
            exit(1);
        }
        printf("Acknowledgement sent: %d\n", packet.sequenceNo);
    }
    char aggregated[1024];
    int offset = 0;
    for (int i = 0; i < chunkCount; i++)
    {
        strncpy(aggregated + offset, packets[i].data, CHUNK_SIZE);
        offset += CHUNK_SIZE;
    }

    printf("[+]Data received: %s\n", aggregated);
    return 0;
}
