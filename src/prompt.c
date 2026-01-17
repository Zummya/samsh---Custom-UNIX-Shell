#define _POSIX_C_SOURCE 200809L
#include "../include/shell.h"
#include "../include/prompt.h"
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

void print_prompt(const char *username, const char *hostname, const char *homeDir) {
    char cwd[4096];
    getcwd(cwd, sizeof(cwd));
    
    // Replace home directory with ~
    char *display_path;
    if (strncmp(cwd, homeDir, strlen(homeDir)) == 0) {
        if (strlen(cwd) == strlen(homeDir)) {
            display_path = "~";
        } else {
            size_t home_len = strlen(homeDir);
            static char path_buf[4096];
            snprintf(path_buf, sizeof(path_buf), "~%s", cwd + home_len);
            display_path = path_buf;
        }
    } else {
        display_path = cwd;
    }
    
    printf("<%s@%s:%s> ", username, hostname, display_path);
    fflush(stdout);
}