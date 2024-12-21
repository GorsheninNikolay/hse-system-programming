
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  FILE* output_fd = fopen("out.txt", "w");
  FILE* error_fd = fopen("err.txt", "w");
  if (argc < 2) {
    fprintf(error_fd, "The first argument is required, it must be path to the tool\n");
  }
  pid_t pid;
  if ((pid = fork()) == 0) {
    // char *arguments[argc - 1];
    // for (int i = 2; i < argc;) {
    //   arguments[i] = argv[i];
    // }
    // execvp(argv[1], arguments);
  }

  fclose(output_fd);
  fclose(error_fd);
  return 0;
}