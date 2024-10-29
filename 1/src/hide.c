#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define HIDDEN_DIR "darkroom/"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Input params must be exaclty one: %s <file name>\n", argv[0]);
    return 1;
  }

  struct stat st = {0};
  if (stat(HIDDEN_DIR, &st) == -1) {
    const mode_t mode = S_IWUSR | S_IXUSR | S_IXGRP | S_IXOTH;  // for darkroom directory
    if (mkdir(HIDDEN_DIR, mode) == -1) {
      fprintf(stderr, "Failed to create hidden directory");
      return 1;
    }
  }

  char new_path[512];
  snprintf(new_path, sizeof(new_path), "%s%s", HIDDEN_DIR, argv[1]);

  if (rename(argv[1], new_path) == -1) {
    fprintf(stderr, "Failed to move file %s\n", argv[1]);
    return 1;
  }

  fprintf(stdout, "File '%s' has been successfully moved in dark directory: '%s'\n", argv[1], new_path);

  return 0;
}
