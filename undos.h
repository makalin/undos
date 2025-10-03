#ifndef UNDOS_H
#define UNDOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <dlfcn.h>
#include <dirent.h>
#include <fcntl.h>

// Maximum path length
#define MAX_PATH_LEN 4096
#define MAX_LOG_LEN 1024

// Default trash directory
#define DEFAULT_TRASH_DIR "~/.local/share/undos_trash"

// Log file name
#define LOG_FILE_NAME "undos.log"

// Function pointer types for original functions
typedef int (*unlink_func_t)(const char *pathname);
typedef int (*unlinkat_func_t)(int dirfd, const char *pathname, int flags);
typedef int (*rmdir_func_t)(const char *pathname);
typedef int (*remove_func_t)(const char *pathname);

// Global variables
extern char trash_dir[MAX_PATH_LEN];
extern char log_file[MAX_PATH_LEN];
extern int undos_initialized;

// Function declarations
void undos_init(void);
int undos_unlink(const char *pathname);
int undos_unlinkat(int dirfd, const char *pathname, int flags);
int undos_rmdir(const char *pathname);
int undos_remove(const char *pathname);
int move_to_trash(const char *pathname);
int create_trash_dir(void);
void log_deletion(const char *original_path, const char *trash_path);
char *expand_path(const char *path);
int is_directory(const char *path);
int copy_file(const char *src, const char *dest);
int copy_directory(const char *src, const char *dest);
void get_timestamp(char *timestamp, size_t size);

#endif // UNDOS_H
