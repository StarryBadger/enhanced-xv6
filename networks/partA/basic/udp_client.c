#include "headers.h"
int main()
{
  int mySocket;
  struct sockaddr_in addr;
  char buffer[1024];
  socklen_t addr_size;

  mySocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (mySocket < 0)
  {
    fprintf(stderr, "[-]Socket creation error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[+]UDP socket created.\n");

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);

  bzero(buffer, 1024);
  printf("Enter message to send to server: ");
  if (scanf("%1023[^\n]", buffer) != 1)
  {
    fprintf(stderr, "[-]Error reading input");
    exit(1);
  }
  if (sendto(mySocket, buffer, 1024, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    fprintf(stderr, "[-]Error while sending data\n");
    exit(1);
  }
  printf("[+]Data sent to server: %s\n", buffer);

  bzero(buffer, 1024);
  addr_size = sizeof(addr);
  if (recvfrom(mySocket, buffer, 1024, 0, (struct sockaddr *)&addr, &addr_size) < 0)
  {
    fprintf(stderr, "[-]Error while receiving data\n");
    exit(1);
  }
  printf("[+]Data received from server: %s\n", buffer);
  return 0;
}