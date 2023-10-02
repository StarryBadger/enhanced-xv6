#include "headers.h"
int main()
{
    int mySocket;
    struct sockaddr_in addr;
    char buffer[1024] = "Hello mother shuckers! It's me";
    socklen_t addr_size;
    int receiveStat, timeSent;
    int ackNumber;

    mySocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mySocket < 0)
    {
        fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+]Socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    int chunkCount = strlen(buffer) / CHUNK_SIZE;
    int finalPacketSize = strlen(buffer) % CHUNK_SIZE;
    if (finalPacketSize)
    {
        ++chunkCount;
    }
    else
    {
        finalPacketSize = CHUNK_SIZE;
    }
    if (sendto(mySocket, &chunkCount, sizeof(chunkCount), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        fprintf(stderr, "[-]Error while sending chunkCount\n");
        exit(1);
    }
    data packets[chunkCount];
    int ackRegister[chunkCount];
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 20;
    setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    struct timeval lastSent[chunkCount];

    for (int i = 0; i < chunkCount; i++)
    {
        packets[i].sequenceNo = i;
        if (i == chunkCount - 1)
        {
            strncpy(packets[i].data, buffer + i * CHUNK_SIZE, finalPacketSize);
            packets[i].data[finalPacketSize] = '\0';
        }
        else
        {
            strncpy(packets[i].data, buffer + i * CHUNK_SIZE, CHUNK_SIZE);
            packets[i].data[CHUNK_SIZE] = '\0';
        }
        ackRegister[i] = 0;
    }
    for (int i = 0; i < chunkCount; i++)
    {
        if (sendto(mySocket, &packets[i], sizeof(packets[i]), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            fprintf(stderr, "[-]Error while sending data chunk %d\n", i);
            exit(1);
        }
        printf("Transmitting... sequence number: %d Data Packet: %s\n", packets[i].sequenceNo, packets[i].data);
        gettimeofday(&lastSent[i], NULL);
        addr_size = sizeof(addr);
        receiveStat = recvfrom(mySocket, &ackNumber, sizeof(ackNumber), 0, (struct sockaddr *)&addr, &addr_size);
        if (receiveStat < 0)
        {
            if (errno != EWOULDBLOCK)
            {

                fprintf(stderr, "[-]Error while receiving data\n");
                exit(1);
            }
        }
        if (receiveStat > 0)
        {
            ackRegister[ackNumber] = 1;
            printf("Ack received: %d\n",ackNumber);
        }
        for (int j = 0; j < i; j++)
        {
            if (!ackRegister[j])
            {
                timeSent = lastSent[j].tv_sec * 1000000 + lastSent[j].tv_usec;
                if (timeSent >= RETRANSMISSION_TIME_USEC)
                {
                    if (sendto(mySocket, &packets[j], sizeof(packets[j]), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                    {
                        fprintf(stderr, "[-]Error while sending data chunk %d\n", j);
                        exit(1);
                    }
                    printf("Retransmitting... sequence number: %d Data Packet: %s\n", packets[j].sequenceNo, packets[j].data);
                    gettimeofday(&lastSent[j], NULL);
                    addr_size = sizeof(addr);
                    receiveStat = recvfrom(mySocket, &ackNumber, sizeof(ackNumber), 0, (struct sockaddr *)&addr, &addr_size);
                    if (receiveStat < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {

                            fprintf(stderr, "[-]Error while receiving data\n");
                            exit(1);
                        }
                    }
                    if (receiveStat > 0)
                    {
                        ackRegister[ackNumber] = 1;
                        printf("Ack received: %d\n",ackNumber);
                    }
                }
            }
        }
    }
    int flag = 1;
    while (flag)
    {
        flag = 0;
        for (int j = 0; j < chunkCount; j++)
        {
            if (!ackRegister[j])
            {
                flag = 1;
                timeSent = lastSent[j].tv_sec * 1000000 + lastSent[j].tv_usec;
                if (timeSent >= RETRANSMISSION_TIME_USEC)
                {
                    if (sendto(mySocket, &packets[j], sizeof(packets[j]), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                    {
                        fprintf(stderr, "[-]Error while sending data chunk %d\n", j);
                        exit(1);
                    }
                    printf("Retransmitting... sequence number: %d Data Packet: %s\n", packets[j].sequenceNo, packets[j].data);
                    gettimeofday(&lastSent[j], NULL);
                    addr_size = sizeof(addr);
                    receiveStat = recvfrom(mySocket, &ackNumber, sizeof(ackNumber), 0, (struct sockaddr *)&addr, &addr_size);
                    if (receiveStat < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {

                            fprintf(stderr, "[-]Error while receiving data\n");
                            exit(1);
                        }
                    }
                    if (receiveStat > 0)
                    {
                        ackRegister[ackNumber] = 1;
                        printf("Ack received: %d\n",ackNumber);
                    }
                }
            }
        }
    }
    return 0;
}
