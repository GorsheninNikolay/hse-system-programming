#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define BROADCAST_IP "255.255.255.255"

void* send_message(void* arg) {
  int socket = *((int*)arg);
  char buffer[BUF_SIZE];
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(addr);

  if (getsockname(socket, (struct sockaddr*)&addr, &len) == -1) {
    fprintf(stderr, "getsockname operation is failed\n");
    close(socket);
    exit(1);
  }
  addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

  while (1) {
    fgets(buffer, BUF_SIZE, stdin);
    if (!strcmp(buffer, "exit\n")) {
      close(socket);
      exit(0);
    }
    sendto(socket, buffer, strlen(buffer), 0, (struct sockaddr*)&addr,
           sizeof(addr));
  }
}

void* recieve_message(void* arg) {
  int socket = *((int*)arg);
  char buffer[BUF_SIZE];
  struct sockaddr_in from_addr;
  socklen_t addr_sz = sizeof(from_addr);

  while (1) {
    int message_length = recvfrom(socket, buffer, BUF_SIZE, 0,
                                  (struct sockaddr*)&from_addr, &addr_sz);
    buffer[message_length] = 0;
    fprintf(stdout, "FROM %s: %s\n", inet_ntoa(from_addr.sin_addr), buffer);
  }
}

int main(int argc, char* argv[]) {
  if (2 != argc) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int udp_socket = 0;
  struct sockaddr_in broadcast_addr;
  memset(&broadcast_addr, 0, sizeof(broadcast_addr));

  udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket == -1) {
    fprintf(stderr, "socket operation is failed\n");
    exit(1);
  }

  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  broadcast_addr.sin_port = htons(atoi(argv[1]));

  int broadcast = 1;
  if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast,
                 sizeof(broadcast)) == -1) {
    fprintf(stderr, "setsockopt operations is failed\n");
    close(udp_socket);
    exit(1);
  }

  if (bind(udp_socket, (struct sockaddr*)&broadcast_addr,
           sizeof(broadcast_addr)) == -1) {
    fprintf(stderr, "bind operation is failed\n");
    close(udp_socket);
    exit(1);
  }

  pthread_t send_thread, recieve_thread;
  void* thread_result;

  pthread_create(&send_thread, NULL, send_message, (void*)&udp_socket);
  pthread_create(&recieve_thread, NULL, recieve_message, (void*)&udp_socket);

  pthread_join(send_thread, &thread_result);
  pthread_join(recieve_thread, &thread_result);

  close(udp_socket);

  exit(0);
}
