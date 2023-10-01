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

  bzero(buffer, 1024);
  addr_size = sizeof(client_addr);
  if (recvfrom(mySocket, buffer, 1024, 0, (struct sockaddr *)&client_addr, &addr_size) < 0)
  {
    fprintf(stderr, "[-]Error while receiving data\n");
    exit(1);
  }
  printf("[+]Data received: %s\n", buffer);

  bzero(buffer, 1024);
  printf("Enter message to send back to client: ");
  if (scanf("%1023[^\n]", buffer) != 1)
  {
    fprintf(stderr, "[-]Error reading input");
    exit(1);
  }
  if (sendto(mySocket, buffer, 1024, 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
  {
    fprintf(stderr, "[-]Error while sending data\n");
    exit(1);
  }
  printf("[+]Data sent: %s\n", buffer);

  return 0;
}