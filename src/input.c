#define _POSIX_C_SOURCE 200809L
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_line(void) {
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t len = getline(&line, &bufsize, stdin);
    
    if (len == -1) {
        if (line) free(line);
        return NULL;
    }
    
    // Remove trailing newline
    if (len > 0 && line[len-1] == '\n') {
        line[len-1] = '\0';
    }
    
    return line;
}
