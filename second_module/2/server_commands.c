#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  if (2 != argc) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int simpleSocket, result = 0;
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));

  simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (simpleSocket == -1) {
    fprintf(stderr, "socket operation is failed\n");
    exit(1);
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(atoi(argv[1]));

  result = bind(simpleSocket, (struct sockaddr*)&server, sizeof(server));
  if (result == -1) {
    fprintf(stderr, "bind operation is failed\n");
    close(simpleSocket);
    exit(1);
  }

  result = listen(simpleSocket, 5);
  if (result == -1) {
    fprintf(stderr, "listen operation is failed\n");
    close(simpleSocket);
    exit(1);
  }

  while (1) {
    struct sockaddr_in clientName = {0};
    int simpleChildSocket = 0;
    unsigned int clientNameLength = sizeof(clientName);

    simpleChildSocket =
        accept(simpleSocket, (struct sockaddr*)&clientName, &clientNameLength);
    if (simpleChildSocket == -1) {
      fprintf(stderr, "accept operation is failed\n");
      close(simpleSocket);
      exit(1);
    }

    
  }

  exit(0);
}
