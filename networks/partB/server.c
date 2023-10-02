#include "headers.h"
int main()
{
    int mySocket;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024]="Hello, I'll be your server for the evening.";
    socklen_t addr_size;
    int n;
    int receiveStat, timeSent;
    int ackNumber;
    struct timeval read_timeout;
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
    int chunkCount, dataflowinfo;

    while (1)
    {
        read_timeout.tv_sec = 100;
        read_timeout.tv_usec = 0;
        setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
        dataflowinfo = -1;
        addr_size = sizeof(client_addr);
        if (recvfrom(mySocket, &dataflowinfo, sizeof(dataflowinfo), 0, (struct sockaddr *)&client_addr, &addr_size) < 0)
        {
            fprintf(stderr, "[-]Error while receiving choice\n");
            exit(1);
        }
        if (dataflowinfo == -1)
        {
            break;
        }
        else if (dataflowinfo == 0)
        {
            if (recvfrom(mySocket, &chunkCount, sizeof(chunkCount), 0, (struct sockaddr *)&client_addr, &addr_size) < 0)
            {
                fprintf(stderr, "[-]Error while receiving chunkCount\n");
                exit(1);
            }
            printf("To receive: %d packets of data\n", chunkCount);

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
        }
        else
        {
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
            if (sendto(mySocket, &chunkCount, sizeof(chunkCount), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
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
                if (sendto(mySocket, &packets[i], sizeof(packets[i]), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
                {
                    fprintf(stderr, "[-]Error while sending data chunk %d\n", i);
                    exit(1);
                }
                printf("Transmitting... sequence number: %d Data Packet: %s\n", packets[i].sequenceNo, packets[i].data);
                gettimeofday(&lastSent[i], NULL);
                addr_size = sizeof(client_addr);
                receiveStat = recvfrom(mySocket, &ackNumber, sizeof(ackNumber), 0, (struct sockaddr *)&client_addr, &addr_size);
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
                            if (sendto(mySocket, &packets[j], sizeof(packets[j]), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
                            {
                                fprintf(stderr, "[-]Error while sending data chunk %d\n", j);
                                exit(1);
                            }
                            printf("Retransmitting... sequence number: %d Data Packet: %s\n", packets[j].sequenceNo, packets[j].data);
                            gettimeofday(&lastSent[j], NULL);
                            addr_size = sizeof(client_addr);
                            receiveStat = recvfrom(mySocket, &ackNumber, sizeof(ackNumber), 0, (struct sockaddr *)&client_addr, &addr_size);
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
                            if (sendto(mySocket, &packets[j], sizeof(packets[j]), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
                            {
                                fprintf(stderr, "[-]Error while sending data chunk %d\n", j);
                                exit(1);
                            }
                            printf("Retransmitting... sequence number: %d Data Packet: %s\n", packets[j].sequenceNo, packets[j].data);
                            gettimeofday(&lastSent[j], NULL);
                            addr_size = sizeof(client_addr);
                            receiveStat = recvfrom(mySocket, &ackNumber, sizeof(ackNumber), 0, (struct sockaddr *)&client_addr, &addr_size);
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
    }
    printf("Data exchange terminated\n");
    return 0;
}
