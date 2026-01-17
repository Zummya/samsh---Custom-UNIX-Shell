#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

static int is_valid_name(const char *str, int start, int end) {
    for (int i = start; i < end; i++) {
        char c = str[i];
        if (c == '|' || c == '&' || c == '>' || c == '<' || c == ';' || isspace(c)) {
            return 0;
        }
    }
    return end > start;
}
// ############## LLM Generated Code Begins ##############
static void skip_whitespace(const char *str, int *pos, int len) {
    while (*pos < len && isspace(str[*pos])) {
        (*pos)++;
    }
}
// ############## LLM Generated Code Ends ##############
// ############## LLM Generated Code Begins ##############
static int parse_redirection(const char *str, int *pos, int len) {
    skip_whitespace(str, pos, len);
    
    if (*pos >= len) return 0;
    
    if (str[*pos] == '<') {
        (*pos)++;
        skip_whitespace(str, pos, len);
        int start = *pos;
        while (*pos < len && !isspace(str[*pos]) && str[*pos] != '|' && 
               str[*pos] != '&' && str[*pos] != '>' && str[*pos] != '<' && str[*pos] != ';') {
            (*pos)++;
        }
        return is_valid_name(str, start, *pos);
    }
    
    if (str[*pos] == '>') {
        (*pos)++;
        if (*pos < len && str[*pos] == '>') {
            (*pos)++;
        }
        skip_whitespace(str, pos, len);
        int start = *pos;
        while (*pos < len && !isspace(str[*pos]) && str[*pos] != '|' && 
               str[*pos] != '&' && str[*pos] != '>' && str[*pos] != '<' && str[*pos] != ';') {
            (*pos)++;
        }
        return is_valid_name(str, start, *pos);
    }
    
    return 1;
}
// ############## LLM Generated Code Ends ##############
static int parse_atomic(const char *str, int *pos, int len) {
    skip_whitespace(str, pos, len);
    
    if (*pos >= len) return 0;
    
    // Parse command name
    int start = *pos;
    while (*pos < len && !isspace(str[*pos]) && str[*pos] != '|' && 
           str[*pos] != '&' && str[*pos] != '>' && str[*pos] != '<' && str[*pos] != ';') {
        (*pos)++;
    }
    
    if (!is_valid_name(str, start, *pos)) return 0;
    
    // Parse arguments and redirections
    while (*pos < len) {
        skip_whitespace(str, pos, len);
        
        if (*pos >= len || str[*pos] == '|' || str[*pos] == '&' || str[*pos] == ';') {
            break;
        }
        
        if (str[*pos] == '<' || str[*pos] == '>') {
            if (!parse_redirection(str, pos, len)) return 0;
        } else {
            // Parse regular argument
            start = *pos;
            while (*pos < len && !isspace(str[*pos]) && str[*pos] != '|' && 
                   str[*pos] != '&' && str[*pos] != '>' && str[*pos] != '<' && str[*pos] != ';') {
                (*pos)++;
            }
            if (!is_valid_name(str, start, *pos)) return 0;
        }
    }
    
    return 1;
}

static int parse_cmd_group(const char *str, int *pos, int len) {
    if (!parse_atomic(str, pos, len)) return 0;
    
    while (*pos < len) {
        skip_whitespace(str, pos, len);
        
        if (*pos >= len || str[*pos] != '|') break;
        
        (*pos)++; // Skip '|'
        
        if (!parse_atomic(str, pos, len)) return 0;
    }
    
    return 1;
}

int is_valid_command(const char *command) {
    if (!command) return 0;
    
    int len = strlen(command);
    int pos = 0;
    
    skip_whitespace(command, &pos, len);
    if (pos >= len) return 1; // Empty command is valid
    
    if (!parse_cmd_group(command, &pos, len)) return 0;
    
    while (pos < len) {
        skip_whitespace(command, &pos, len);
        
        if (pos >= len) break;
        
        if (command[pos] == '&') {
            pos++;
            skip_whitespace(command, &pos, len);
            if (pos < len && command[pos] != '&' && command[pos] != ';') {
                return 0; // & must be at end or followed by whitespace
            }
            if (pos >= len) break; // & at end is valid
        }
        
        if (pos < len && command[pos] == ';') {
            pos++;
            skip_whitespace(command, &pos, len);
            if (pos >= len) break; // ; at end is valid
            
            if (!parse_cmd_group(command, &pos, len)) return 0;
        } else if (pos < len) {
            return 0; // Invalid character
        }
    }
    
    return 1;
}
