#define _GNU_SOURCE
#include "undos.h"

// Global variables
char trash_dir[MAX_PATH_LEN] = {0};
char log_file[MAX_PATH_LEN] = {0};
int undos_initialized = 0;

// Function pointers for original functions
static unlink_func_t original_unlink = NULL;
static unlinkat_func_t original_unlinkat = NULL;
static rmdir_func_t original_rmdir = NULL;
static remove_func_t original_remove = NULL;

// Initialize undos
void undos_init(void) {
    if (undos_initialized) return;
    
    // Get trash directory from environment or use default
    const char *env_trash = getenv("UNDOS_TRASH");
    if (env_trash) {
        strncpy(trash_dir, env_trash, MAX_PATH_LEN - 1);
    } else {
        strncpy(trash_dir, DEFAULT_TRASH_DIR, MAX_PATH_LEN - 1);
    }
    
    // Expand ~ in path
    char *expanded = expand_path(trash_dir);
    if (expanded) {
        strncpy(trash_dir, expanded, MAX_PATH_LEN - 1);
        free(expanded);
    }
    
    // Set up log file path
    snprintf(log_file, MAX_PATH_LEN, "%s/%s", trash_dir, LOG_FILE_NAME);
    
    // Create trash directory if it doesn't exist
    create_trash_dir();
    
    undos_initialized = 1;
}

// Get original function pointers
static void get_original_functions(void) {
    if (original_unlink) return; // Already initialized
    
    original_unlink = (unlink_func_t)dlsym(RTLD_NEXT, "unlink");
    original_unlinkat = (unlinkat_func_t)dlsym(RTLD_NEXT, "unlinkat");
    original_rmdir = (rmdir_func_t)dlsym(RTLD_NEXT, "rmdir");
    original_remove = (remove_func_t)dlsym(RTLD_NEXT, "remove");
}

// Override unlink function
int unlink(const char *pathname) {
    if (!undos_initialized) undos_init();
    if (!original_unlink) get_original_functions();
    
    // Don't intercept if it's our own trash directory
    if (strstr(pathname, trash_dir) == pathname) {
        return original_unlink(pathname);
    }
    
    // Move to trash instead of deleting
    if (move_to_trash(pathname) == 0) {
        return 0; // Success
    }
    
    // Fallback to original unlink if moving to trash fails
    return original_unlink(pathname);
}

// Override unlinkat function
int unlinkat(int dirfd, const char *pathname, int flags) {
    if (!undos_initialized) undos_init();
    if (!original_unlinkat) get_original_functions();
    
    // Don't intercept if it's our own trash directory
    if (strstr(pathname, trash_dir) == pathname) {
        return original_unlinkat(dirfd, pathname, flags);
    }
    
    // For unlinkat, we need to construct the full path
    char full_path[MAX_PATH_LEN];
    if (pathname[0] == '/') {
        // Absolute path
        strncpy(full_path, pathname, MAX_PATH_LEN - 1);
    } else {
        // Relative path - get current directory
        char cwd[MAX_PATH_LEN];
        if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
            return original_unlinkat(dirfd, pathname, flags);
        }
        snprintf(full_path, MAX_PATH_LEN, "%s/%s", cwd, pathname);
    }
    
    // Move to trash instead of deleting
    if (move_to_trash(full_path) == 0) {
        return 0; // Success
    }
    
    // Fallback to original unlinkat if moving to trash fails
    return original_unlinkat(dirfd, pathname, flags);
}

// Override rmdir function
int rmdir(const char *pathname) {
    if (!undos_initialized) undos_init();
    if (!original_rmdir) get_original_functions();
    
    // Don't intercept if it's our own trash directory
    if (strstr(pathname, trash_dir) == pathname) {
        return original_rmdir(pathname);
    }
    
    // Move directory to trash instead of deleting
    if (move_to_trash(pathname) == 0) {
        return 0; // Success
    }
    
    // Fallback to original rmdir if moving to trash fails
    return original_rmdir(pathname);
}

// Override remove function
int remove(const char *pathname) {
    if (!undos_initialized) undos_init();
    if (!original_remove) get_original_functions();
    
    // Don't intercept if it's our own trash directory
    if (strstr(pathname, trash_dir) == pathname) {
        return original_remove(pathname);
    }
    
    // Move to trash instead of deleting
    if (move_to_trash(pathname) == 0) {
        return 0; // Success
    }
    
    // Fallback to original remove if moving to trash fails
    return original_remove(pathname);
}

// Move file/directory to trash
int move_to_trash(const char *pathname) {
    if (!pathname || strlen(pathname) == 0) return -1;
    
    // Check if path exists
    if (access(pathname, F_OK) != 0) {
        return -1; // File doesn't exist
    }
    
    // Create trash directory if it doesn't exist
    if (create_trash_dir() != 0) {
        return -1;
    }
    
    // Generate unique name for trash
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    // Get basename of the file
    const char *basename = strrchr(pathname, '/');
    basename = basename ? basename + 1 : pathname;
    
    // Create unique name: timestamp_basename
    char trash_name[MAX_PATH_LEN];
    snprintf(trash_name, MAX_PATH_LEN, "%s_%s", timestamp, basename);
    
    char trash_path[MAX_PATH_LEN];
    snprintf(trash_path, MAX_PATH_LEN, "%s/%s", trash_dir, trash_name);
    
    // Copy file/directory to trash
    int result;
    if (is_directory(pathname)) {
        result = copy_directory(pathname, trash_path);
    } else {
        result = copy_file(pathname, trash_path);
    }
    
    if (result == 0) {
        // Log the deletion
        log_deletion(pathname, trash_path);
        
        // Remove original file/directory
        if (is_directory(pathname)) {
            original_rmdir(pathname);
        } else {
            original_unlink(pathname);
        }
    }
    
    return result;
}

// Create trash directory
int create_trash_dir(void) {
    struct stat st;
    if (stat(trash_dir, &st) == 0) {
        return 0; // Directory already exists
    }
    
    // Create directory recursively
    char *path = strdup(trash_dir);
    char *p = path;
    
    // Skip leading slashes
    while (*p == '/') p++;
    
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(path, 0755) != 0 && errno != EEXIST) {
                free(path);
                return -1;
            }
            *p = '/';
        }
        p++;
    }
    
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        free(path);
        return -1;
    }
    
    free(path);
    return 0;
}

// Log deletion
void log_deletion(const char *original_path, const char *trash_path) {
    FILE *fp = fopen(log_file, "a");
    if (!fp) return;
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    fprintf(fp, "%s|%s|%s\n", timestamp, original_path, trash_path);
    fclose(fp);
}

// Expand ~ in path
char *expand_path(const char *path) {
    if (!path) return NULL;
    
    if (path[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) return strdup(path);
        
        size_t home_len = strlen(home);
        size_t path_len = strlen(path);
        char *expanded = malloc(home_len + path_len);
        if (!expanded) return NULL;
        
        strcpy(expanded, home);
        strcat(expanded, path + 1);
        return expanded;
    }
    
    return strdup(path);
}

// Check if path is a directory
int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

// Copy file
int copy_file(const char *src, const char *dest) {
    FILE *src_fp = fopen(src, "rb");
    if (!src_fp) return -1;
    
    FILE *dest_fp = fopen(dest, "wb");
    if (!dest_fp) {
        fclose(src_fp);
        return -1;
    }
    
    char buffer[8192];
    size_t bytes_read;
    int result = 0;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_fp)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest_fp) != bytes_read) {
            result = -1;
            break;
        }
    }
    
    fclose(src_fp);
    fclose(dest_fp);
    
    return result;
}

// Copy directory recursively
int copy_directory(const char *src, const char *dest) {
    if (mkdir(dest, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    
    DIR *dir = opendir(src);
    if (!dir) return -1;
    
    struct dirent *entry;
    int result = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char src_path[MAX_PATH_LEN];
        char dest_path[MAX_PATH_LEN];
        
        snprintf(src_path, MAX_PATH_LEN, "%s/%s", src, entry->d_name);
        snprintf(dest_path, MAX_PATH_LEN, "%s/%s", dest, entry->d_name);
        
        if (is_directory(src_path)) {
            result = copy_directory(src_path, dest_path);
        } else {
            result = copy_file(src_path, dest_path);
        }
        
        if (result != 0) break;
    }
    
    closedir(dir);
    return result;
}

// Get current timestamp
void get_timestamp(char *timestamp, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, size, "%Y%m%d_%H%M%S", tm_info);
}
