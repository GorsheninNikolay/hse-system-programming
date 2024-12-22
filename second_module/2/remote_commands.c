#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TIMEOUT 2
#define MAX_ARGS 128
#define BUFFER_SIZE 1024

pid_t current_pid = 0;

void timeout_handler(int sig) {
  fprintf(stderr, "Timeout occurred\n");
  kill(current_pid, SIGALRM);
}

void prepare_command(char *argv_buffer[], char buffer[], unsigned int length) {
  buffer[length] = '\0';
  char *newline = strchr(buffer, '\n');
  if (newline) {
    *newline = '\0';
  }

  int arg_length = 0;
  char *token;

  token = strtok(buffer, " ");
  while (token != NULL && arg_length < MAX_ARGS - 1) {
    argv_buffer[arg_length++] = token;
    token = strtok(NULL, " ");
  }
  argv_buffer[arg_length] = NULL;
}

int main(int argc, char *argv[]) {
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

  result = bind(simpleSocket, (struct sockaddr *)&server, sizeof(server));
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
        accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

    if (simpleChildSocket == -1) {
      fprintf(stderr, "accept operation is failed\n");
      close(simpleSocket);
      exit(1);
    }

    unsigned int length = 0;
    char buffer[BUFFER_SIZE];
    if ((length = recv(simpleChildSocket, buffer, sizeof(buffer), 0)) > 0) {
      char *argv_buffer[MAX_ARGS];
      prepare_command(argv_buffer, buffer, length);

      int local_result = 0;
      pid_t pid = fork();
      if (pid == -1) {
        exit(1);
      } else if (pid == 0) {
        dup2(simpleChildSocket, STDOUT_FILENO);
        dup2(simpleChildSocket, STDERR_FILENO);

        local_result = execvp(argv_buffer[0], argv_buffer);
      } else {
        current_pid = pid;

        signal(SIGALRM, timeout_handler);
        alarm(TIMEOUT);
        if (waitpid(pid, &local_result, 0) == -1) {
          exit(1);
        }
        dprintf(simpleChildSocket, "exit status: %d\n", local_result);

        current_pid = 0;
        alarm(0);
      }
    }
    close(simpleChildSocket);
    printf("HELLO!");
  }
  exit(0);
}
