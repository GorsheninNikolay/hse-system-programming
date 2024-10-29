#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define STATIC_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define FILE_TYPES_COUNT 8
#define CURRENT_PATH "."

typedef struct {
  size_t regular_files;
  size_t directories;
  size_t character_files;
  size_t block_files;
  size_t fifos_or_pipes;
  size_t symbolic_links;
  size_t sockets;
  size_t unknown;
} FilesStat;

void countFileStat(const char* path, FilesStat* filestat) {
  struct stat current_file_stat;
  if (stat(path, &current_file_stat) < 0) {
    fprintf(stderr, "Failed to get info about file %s\n", path);
    return;
  }

  const mode_t mode = current_file_stat.st_mode;
  size_t* stats[FILE_TYPES_COUNT] = {
      &filestat->regular_files,     &filestat->directories,
      &filestat->character_files, &filestat->block_files,
      &filestat->fifos_or_pipes,    &filestat->symbolic_links,
      &filestat->sockets,           &filestat->unknown};

  for (int i = 0; i < FILE_TYPES_COUNT - 1; i++) {
    if ((i == 0 && S_ISREG(mode)) || (i == 1 && S_ISDIR(mode)) ||
        (i == 2 && S_ISCHR(mode)) || (i == 3 && S_ISBLK(mode)) ||
        (i == 4 && S_ISFIFO(mode)) || (i == 5 && S_ISLNK(mode)) ||
        (i == 6 && S_ISSOCK(mode))) {
      ++(*stats[i]);
      return;
    }
  }

  ++(*stats[7]);
}

int isPathInException(const char* path) {
  static const char* except_paths[] = {
      ".", ".."};  // Should we except this default paths? I think yes
  for (int i = 0; i < STATIC_ARRAY_SIZE(except_paths); i++) {
    if (strcmp(except_paths[i], path) == 0) {
      return 1;
    }
  }
  return 0;
}

int main() {
  DIR* dir;
  static const char* current_path = CURRENT_PATH;

  dir = opendir(current_path);
  if (dir) {
    struct dirent* dir_info;
    FilesStat files_stat = {0};
    while ((dir_info = readdir(dir)) != NULL) {
      const char* path = dir_info->d_name;
      if (!isPathInException(path)) {
        fprintf(stdout, "checking path %s\n", path);
        countFileStat(path, &files_stat);
      }
    }
    fprintf(stdout, "Regular files: %zu\n", files_stat.regular_files);
    fprintf(stdout, "Directories: %zu\n", files_stat.directories);
    fprintf(stdout, "Character files: %zu\n", files_stat.character_files);
    fprintf(stdout, "Block files: %zu\n", files_stat.block_files);
    fprintf(stdout, "FIFOs/Pipes: %zu\n", files_stat.fifos_or_pipes);
    fprintf(stdout, "Symbolic links: %zu\n", files_stat.symbolic_links);
    fprintf(stdout, "Sockets: %zu\n", files_stat.sockets);
    fprintf(stdout, "Unknown type: %zu\n", files_stat.unknown);

  } else /* unexpected case with const path "." */ {
    fprintf(stderr, "Failed to open directory by %s path\n", current_path);
  }
  closedir(dir);
  return 0;
}
