#include "headers.h"
int myatoi(char *str)
{
    if (str[1] != '\0')
        return -1;
    if (str[0] == '0')
        return -2; // send
    else if (str[0] == '1')
        return -3; // receive
    return -1;
}
void removenextline(char *buffer)
{
    int i = 0;
    while (buffer[i] != '\0')
    {
        ++i;
    }
    if (buffer[i - 1] == '\n')
    {
        buffer[i - 1] = '\0';
    }
}
int main()
{
    int mySocket;
    struct sockaddr_in addr;
    char num[10];
    socklen_t addr_size;
    int receiveStat, timeSent;
    int ackNumber;
    int chunkCount;
    struct timeval read_timeout;

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

    while (1)
    {
        read_timeout.tv_sec = 100;
        read_timeout.tv_usec = 0;
        char buffer[1024];
        printf("Enter 0 to send data to server, 1 to receive data and any other key to stop\n");
        if (scanf("%s", num) != 1)
        {
            fprintf(stderr, "[-]Error reading input");
            exit(1);
        }
        getchar();
        int x = myatoi(num);
        if (sendto(mySocket, &x, sizeof(x), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            fprintf(stderr, "[-]Error while sending choice\n");
            exit(1);
        }
        if (x == -1)
        {
            break;
        }
        else if (x == -2)
        {
            setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
            memset(&buffer, '\0', sizeof(buffer));
            printf("Enter message to send: ");
            fgets(buffer, 1024, stdin);
            removenextline(buffer);
            chunkCount = strlen(buffer) / CHUNK_SIZE;
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
            int sentRegister[chunkCount];
            int ackRegister[chunkCount];
            int ackedChunkCount = 0;
            read_timeout.tv_sec = 0;
            read_timeout.tv_usec = 1000;
            setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

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
                sentRegister[i] = 0;
            }
            while (ackedChunkCount != chunkCount)
            {
                for (int i = 0; i < chunkCount; i++)
                {
                    if (ackRegister[i])
                        continue;
                    if (sendto(mySocket, &packets[i], sizeof(packets[i]), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                    {
                        fprintf(stderr, "[-]Error while sending data chunk %d\n", i);
                        exit(1);
                    }
                    if (sentRegister[i])
                    {
                        printf("Retransmitting... sequence number: %d Data Packet: %s\n", packets[i].sequenceNo, packets[i].data);
                    }
                    else
                    {
                        sentRegister[i] = 1;
                        printf("Transmitting... sequence number: %d Data Packet: %s\n", packets[i].sequenceNo, packets[i].data);
                    }
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
                        if (ackRegister[ackNumber] == 0)
                        {
                            ++ackedChunkCount;
                            ackRegister[ackNumber] = 1;
                        }
                        printf("Ack received: %d\n", ackNumber);
                    }
                }
            }
        }

        else if (x == -3)
        {
            addr_size = sizeof(addr);
            setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
            if (recvfrom(mySocket, &chunkCount, sizeof(chunkCount), 0, (struct sockaddr *)&addr, &addr_size) < 0)
            {
                if ((errno != EWOULDBLOCK))
                {
                    fprintf(stderr, "[-]Error while receiving chunkCount\n");
                }
                else
                {
                    printf("Response took too long\n");
                }
                exit(1);
            }
            printf("To receive: %d packets of data\n", chunkCount);

            struct timeval read_timeout;
            read_timeout.tv_sec = 0;
            read_timeout.tv_usec = 1000;
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
                addr_size = sizeof(addr);
                if (recvfrom(mySocket, &packet, sizeof(packet), 0, NULL, NULL) < 0 && (errno != EWOULDBLOCK))
                {
                    fprintf(stderr, "[-]Error while receiving data chunk\n");
                    exit(1);
                }
                // printf("Sequence number: %d Data Packet: %s\n", packet.sequenceNo, packet.data);
                // if ((packet.sequenceNo % 3 != 0) && flag[packet.sequenceNo] < 10)
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

                if (sendto(mySocket, &packet.sequenceNo, sizeof(packet.sequenceNo), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                {
                    fprintf(stderr, "[-]Error while sending acknowledgement\n");
                    exit(1);
                }
                // printf("Acknowledgement sent: %d\n", packet.sequenceNo);
            }
            char aggregated[1024];
            memset(&aggregated, '\0', sizeof(aggregated));
            int offset = 0;
            for (int i = 0; i < chunkCount; i++)
            {
                strncpy(aggregated + offset, packets[i].data, CHUNK_SIZE);
                offset += CHUNK_SIZE;
            }

            printf("[+]Data received: %s\n", aggregated);
        }
    }
    printf("Data exchange terminated\n");
    return 0;
}
