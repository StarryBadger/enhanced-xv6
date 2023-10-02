#include "headers.h"
int myatoi(char *str)
{
    if (str[1] != '\0')
        return -1;
    if (str[0] == '0')
        return 0;
    else if (str[0] == '1')
        return 1;
    return -1;
}
int main()
{
    int mySocket;
    struct sockaddr_in addr;
    char buffer[1024] = "Hello server! It's me.";
    char num[10];
    socklen_t addr_size;
    int receiveStat, timeSent;
    int ackNumber;
    int chunkCount;
    int x = 0;
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
        printf("Enter 0 to send data to server, 1 to receive data and any other key to stop\n");
        if (scanf("%s", num) != 1)
        {
            fprintf(stderr, "[-]Error reading input");
            exit(1);
        }
        getchar();
        x = myatoi(num);
        if (sendto(mySocket, &x, sizeof(x), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            fprintf(stderr, "[-]Error while sending choice\n");
            exit(1);
        }
        if (x == -1)
        {
            break;
        }
        else if (x == 0)
        {
            setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
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
            int ackRegister[chunkCount];
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
                    printf("Ack received: %d\n", ackNumber);
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
                                printf("Ack received: %d\n", ackNumber);
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
                                printf("Ack received: %d\n", ackNumber);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (recvfrom(mySocket, &chunkCount, sizeof(chunkCount), 0, (struct sockaddr *)&addr, &addr_size) < 0)
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

            if (sendto(mySocket, &packet.sequenceNo, sizeof(packet.sequenceNo), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
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
        }
    }
    printf("Data exchange terminated\n");
    return 0;
}
