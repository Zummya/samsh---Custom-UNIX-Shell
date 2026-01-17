#define _POSIX_C_SOURCE 200809L
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

static void log_load(const char *logfile, char **log, int *log_count) {
    *log_count = 0;
    FILE *fp = fopen(logfile, "r");
    if (!fp) return;
    
    char buf[4096];
    while (*log_count < LOG_MAX && fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
        log[(*log_count)++] = strdup(buf);
    }
    fclose(fp);
}

static void log_save(const char *logfile, char **log, int log_count) {
    FILE *fp = fopen(logfile, "w");
    if (!fp) return;
    
    for (int i = 0; i < log_count; i++) {
        fprintf(fp, "%s\n", log[i]);
    }
    fclose(fp);
}
// ############## LLM Generated Code Begins ##############
void builtin_hop(int argc, char **argv, const char *homeDir, char **prevDir) {
    char cwd[4096];
    getcwd(cwd, sizeof(cwd));
    
    // Handle no arguments case
    if (argc == 1) {
        if (chdir(homeDir) == 0) {
            if (*prevDir) free(*prevDir);
            *prevDir = strdup(cwd);
        } else {
            printf("No such directory!\n");
        }
        return;
    }
    
    for (int i = 1; i < argc; i++) {
        char *target = argv[i];
        char *newDir = NULL;
        
        if (strcmp(target, "~") == 0) {
            newDir = strdup(homeDir);
        } else if (strcmp(target, ".") == 0) {
            continue;  // Do nothing for current directory
        } else if (strcmp(target, "..") == 0) {
            char *parent = strrchr(cwd, '/');
            if (parent && parent != cwd) {
                *parent = '\0';
                newDir = strdup(cwd);
                *parent = '/';
            } else {
                newDir = strdup("/");
            }
        } else if (strcmp(target, "-") == 0) {
            if (*prevDir) {
                newDir = strdup(*prevDir);
            } else {
                printf("No such directory!\n");
                continue;
            }
        } else {
            newDir = strdup(target);
        }
        
        if (newDir && chdir(newDir) == 0) {
            if (*prevDir) free(*prevDir);
            *prevDir = strdup(cwd);
            getcwd(cwd, sizeof(cwd));
        } else {
            printf("No such directory!\n");
        }
        
        if (newDir) free(newDir);
    }
}

static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void builtin_reveal(int argc, char **argv, const char *homeDir, char **prevDir) {
    int show_hidden = 0, long_format = 0;
    char *target_dir = ".";
    int arg_count = 0;
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j]; j++) {
                if (argv[i][j] == 'a') show_hidden = 1;
                else if (argv[i][j] == 'l') long_format = 1;
            }
        } else {
            if (arg_count > 0) {
                printf("reveal: Invalid Syntax!\n");
                return;
            }
            target_dir = argv[i];
            arg_count++;
        }
    }
    
    // Handle special directories
    char *actual_dir = target_dir;
    if (strcmp(target_dir, "~") == 0) {
        actual_dir = (char*)homeDir;
    } else if (strcmp(target_dir, "-") == 0) {
        if (!*prevDir || strlen(*prevDir) == 0) {
            printf("No such directory!\n");
            return;
        }
        actual_dir = *prevDir;
    }
    
    DIR *dir = opendir(actual_dir);
    if (!dir) {
        printf("No such directory!\n");
        return;
    }
    
    char *files[1024];
    int file_count = 0;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) && file_count < 1024) {
        if (!show_hidden && entry->d_name[0] == '.') continue;
        files[file_count++] = strdup(entry->d_name);
    }
    closedir(dir);
    
    qsort(files, file_count, sizeof(char*), compare_strings);
    
    if (long_format) {
        for (int i = 0; i < file_count; i++) {
            printf("%s\n", files[i]);
        }
    } else {
        for (int i = 0; i < file_count; i++) {
            printf("%s", files[i]);
            if (i < file_count - 1) printf(" ");
        }
        if (file_count > 0) printf("\n");
    }
    
    for (int i = 0; i < file_count; i++) {
        free(files[i]);
    }
}

int builtin_log(int argc, char **argv, const char *logfile, int skip_store, const char *last_cmd) {
    char *log[LOG_MAX+1];
    int log_count = 0;
    log_load(logfile, log, &log_count);
    
    if (!skip_store && last_cmd && (!log_count || strcmp(last_cmd, log[log_count-1]) != 0)) {
        if (log_count == LOG_MAX) {
            free(log[0]);
            memmove(log, log+1, (LOG_MAX-1)*sizeof(char*));
            log_count--;
        }
        log[log_count++] = strdup(last_cmd);
        log_save(logfile, log, log_count);
    }
    
    if (argc == 1) {
        for (int i = 0; i < log_count; i++) {
            printf("%s\n", log[i]);
        }
    } else if (argc == 2 && strcmp(argv[1], "purge") == 0) {
        log_save(logfile, log, 0);
    } else if (argc == 3 && strcmp(argv[1], "execute") == 0) {
        int idx = atoi(argv[2]);
        if (idx < 1 || idx > log_count) {
            for (int i = 0; i < log_count; i++) free(log[i]);
            printf("log: Invalid Syntax!\n");
            return 1;
        }
        printf("%s\n", log[log_count-idx]);
    } else {
        printf("log: Invalid Syntax!\n");
    }
    
    for (int i = 0; i < log_count; i++) free(log[i]);
    return 0;
}

int log_store_only(const char *logfile, const char *cmd) {
    char *log[16];
    FILE *fp = fopen(logfile, "a+");
    if (!fp) return 1;
    rewind(fp);
    
    char buf[4096];
    int n = 0;
    while (n < 15 && fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = 0;
        log[n++] = strdup(buf);
    }
    
    if (n == 0 || strcmp(cmd, log[n-1]) != 0) {
        if (n == 15) {
            free(log[0]);
            for (int i = 0; i < n-1; i++) {
                log[i] = log[i+1];
            }
            n--;
        }
        fclose(fp);
        fp = fopen(logfile, "w");
        for (int i = 0; i < n; i++) {
            fprintf(fp, "%s\n", log[i]);
        }
        fprintf(fp, "%s\n", cmd);
    }
// ############## LLM Generated Code Ends ##############
    fclose(fp);
    for (int i = 0; i < n; i++) free(log[i]);
    return 0;
}