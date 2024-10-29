#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAGIC_BYTES_SIZE 128
#define DEFENSE_KEY "STASHED"
#define KEY 0XA9

void xorFile(int fd, const size_t bytes_to_encrypt) {
  char *data;
  lseek(fd, 0, SEEK_SET);
  read(fd, data, bytes_to_encrypt);
  for (int i = 0; i < bytes_to_encrypt; i++) {
    data[i] ^= KEY;
  }
  lseek(fd, 0, SEEK_SET);
  write(fd, data, bytes_to_encrypt);
}

int alterFile(const char *filename, const char *mode) {
  int fd = open(filename, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "File %s is not found\n", filename);
    close(fd);
    return 1;
  }
  size_t defense_key_size = strlen(DEFENSE_KEY);
  off_t end_pos = lseek(fd, 0, SEEK_END);
  size_t bytes_to_encrypt =
      end_pos < MAGIC_BYTES_SIZE ? end_pos : MAGIC_BYTES_SIZE;

  char *buffer;
  lseek(fd, -defense_key_size, SEEK_END);
  read(fd, buffer, defense_key_size);

  if (strcmp(mode, "code") == 0) {
    if (strcmp(buffer, DEFENSE_KEY) == 0) {
      fprintf(stderr, "File has been already stashed, skip...\n");
      return 0;
    }
    xorFile(fd, bytes_to_encrypt);
    lseek(fd, 0, SEEK_END);
    write(fd, DEFENSE_KEY, defense_key_size);
  } else /* if (strcmp(mode, "decode") == 0) */ {
    if (strcmp(buffer, DEFENSE_KEY) != 0) {
      fprintf(stderr, "File has not been stashed, skip...\n");
      return 0;
    }
    xorFile(fd, bytes_to_encrypt);
    ftruncate(fd, end_pos - defense_key_size);
  }
  close(fd);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr,
            "Arguments must be excalty 2, example: %s <mode: code/decode> "
            "<file_name>\n",
            argv[0]);
    return 1;
  }
  const char *mode = argv[1];

  if (strcmp(mode, "code") != 0 && strcmp(mode, "decode") != 0) {
    fprintf(stderr, "Unknown mode %s, must be one of: 'code' or 'decode'\n",
            mode);
    return 1;
  }

  const char *filename = argv[2];
  return alterFile(filename, mode);
}
