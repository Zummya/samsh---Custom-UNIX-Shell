#define _POSIX_C_SOURCE 200809L
#include "shell.h"
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "builtins.h"
#include "jobs.h"
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static char *homeDir = NULL;
static char *prevDir = NULL;
static char *logfile = ".shell_log";

volatile pid_t fg_pid = 0;
char fg_cmd[4096] = "";  // Add this to track foreground command
// ############## LLM Generated Code Begins ##############
void handle_sigint(int sig) {
    (void)sig;
    if (fg_pid > 0) {
        // Send SIGINT to the foreground process group
        kill(-fg_pid, SIGINT);
    } else {
        // Just redisplay prompt if no foreground process
        printf("\n");
        fflush(stdout);
    }
    // Don't exit the shell
}
// ############## LLM Generated Code Ends ##############
// ############## LLM Generated Code Begins ##############
void handle_sigtstp(int sig) {
    (void)sig;
    if (fg_pid > 0) {
        // Send SIGTSTP to the foreground process group
        kill(-fg_pid, SIGTSTP);
        
        // Add the stopped job to the job list
        add_job(fg_pid, fg_cmd, JOB_STOPPED);
        
        // Print the stopped job message
        int job_num = 0;
        job_t *j = job_list;
        while (j) {
            if (j->pid == fg_pid) {
                job_num = j->job_num;
                break;
            }
            j = j->next;
        }
        
        printf("\n[%d] Stopped %s\n", job_num, fg_cmd);
        fflush(stdout);
        
        // Reset foreground process
        fg_pid = 0;
        fg_cmd[0] = '\0';
    } else {
        // Just redisplay prompt if no foreground process
        printf("\n");
        fflush(stdout);
    }
    // Don't stop the shell
}
// ############## LLM Generated Code Ends ##############

static void kill_all_children(void) {
    job_t *j = job_list;
    while (j) {
        if (j->pid > 0) kill(-j->pid, SIGKILL);
        j = j->next;
    }
}

static int split_commands(char *line, char **cmds, char *delims) {
    int count = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(line, delims, &saveptr);
    while (tok && count < 64) {
        cmds[count++] = tok;
        tok = strtok_r(NULL, delims, &saveptr);
    }
    cmds[count] = NULL;
    return count;
}

static int is_background(char *cmd) {
    int len = strlen(cmd);
    while (len > 0 && (cmd[len-1] == ' ' || cmd[len-1] == '\t')) --len;
    if (len > 0 && cmd[len-1] == '&') {
        cmd[len-1] = '\0';
        return 1;
    }
    return 0;
}
// ############## LLM Generated Code Begins ##############
static void execute_command(char *cmd, int bg, char *logfile) {
    // Save the command for foreground processes
    if (!bg) {
        strncpy(fg_cmd, cmd, sizeof(fg_cmd)-1);
        fg_cmd[sizeof(fg_cmd)-1] = '\0';
    }
    
    char *pipes[16];
    int npipes = split_commands(cmd, pipes, "|");
    int pipefds[2][2];
    int last_in = -1;
    pid_t pids[16];
    pid_t pgid = 0;  // Process group for the entire pipeline
    
    for (int i = 0; i < npipes; ++i) {
        if (i < npipes - 1) pipe(pipefds[i%2]);
        
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            
            // Reset signal handlers to default in child
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            
            // Parse redirections - find the LAST occurrence of each type
            char *cmd_copy = strdup(pipes[i]);
            char *infile = NULL, *outfile = NULL;
            int append = 0;
            
            // Find last input redirection
            char *last_input = NULL;
            char *p = cmd_copy;
            while ((p = strchr(p, '<')) != NULL) {
                last_input = p;
                p++;
            }
            
            // Find last output redirection
            char *last_output = NULL;
            int last_is_append = 0;
            p = cmd_copy;
            while ((p = strchr(p, '>')) != NULL) {
                if (p > cmd_copy && *(p-1) == '>') {
                    // This is >>, but we found > first, so skip
                    p++;
                    continue;
                }
                if (*(p+1) == '>') {
                    // This is >>
                    last_output = p;
                    last_is_append = 1;
                    p += 2;
                } else {
                    // This is >
                    last_output = p;
                    last_is_append = 0;
                    p++;
                }
            }
            
            // Process input redirection
            if (last_input) {
                *last_input = '\0';  // Terminate command part
                char *filename = last_input + 1;
                while (*filename == ' ' || *filename == '\t') filename++;
                char *end = filename;
                while (*end && *end != ' ' && *end != '\t' && *end != '>' && *end != '<' && *end != '|') end++;
                *end = '\0';
                infile = strdup(filename);
            }
            
            // Process output redirection
            if (last_output) {
                *last_output = '\0';  // Terminate command part
                char *filename = last_output + (last_is_append ? 2 : 1);
                while (*filename == ' ' || *filename == '\t') filename++;
                char *end = filename;
                while (*end && *end != ' ' && *end != '\t' && *end != '>' && *end != '<' && *end != '|') end++;
                *end = '\0';
                outfile = strdup(filename);
                append = last_is_append;
            }
            
            // Background processes shouldn't read from terminal
            if (bg) {
                close(STDIN_FILENO);
                open("/dev/null", O_RDONLY);
            }
            
            // Handle input redirection
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd < 0) { 
                    fprintf(stderr, "No such file or directory\n"); 
                    exit(1); 
                }
                dup2(fd, STDIN_FILENO); 
                close(fd);
                free(infile);
            }
            
            // Handle output redirection
            if (outfile) {
                int fd = open(outfile, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
                if (fd < 0) { 
                    fprintf(stderr, "Unable to create file for writing\n"); 
                    exit(1); 
                }
                dup2(fd, STDOUT_FILENO); 
                close(fd);
                free(outfile);
            }
            
            // Set up pipe input/output
            if (i > 0) { 
                dup2(last_in, STDIN_FILENO); 
                close(last_in); 
            }
            if (i < npipes - 1) { 
                close(pipefds[i%2][0]); 
                dup2(pipefds[i%2][1], STDOUT_FILENO); 
                close(pipefds[i%2][1]); 
            }
            
            // Parse command arguments
            char *argv[128];
            char *saveptr2 = NULL;
            int argc2 = 0;
            char *tok2 = strtok_r(cmd_copy, " \t", &saveptr2);
            while (tok2 && argc2 < 127) {
                argv[argc2++] = tok2;
                tok2 = strtok_r(NULL, " \t", &saveptr2);
            }
            argv[argc2] = NULL;
            
            // Handle built-in commands in pipeline
            if (argc2 > 0) {
                if (strcmp(argv[0], "hop") == 0) {
                    builtin_hop(argc2, argv, homeDir, &prevDir);
                    exit(0);
                } else if (strcmp(argv[0], "reveal") == 0) {
                    builtin_reveal(argc2, argv, homeDir, &prevDir);
                    exit(0);
                } else if (strcmp(argv[0], "log") == 0) {
                    builtin_log(argc2, argv, logfile, 1, NULL);
                    exit(0);
                } else if (strcmp(argv[0], "activities") == 0) {
                    builtin_activities();
                    exit(0);
                } else if (strcmp(argv[0], "ping") == 0) {
                    builtin_ping(argc2, argv);
                    exit(0);
                } else if (strcmp(argv[0], "fg") == 0) {
                    builtin_fg(argc2, argv);
                    exit(0);
                } else if (strcmp(argv[0], "bg") == 0) {
                    builtin_bg(argc2, argv);
                    exit(0);
                } else if (strcmp(argv[0], "echo") == 0) {
                    for (int j = 1; j < argc2; ++j) {
                        printf("%s%s", argv[j], (j == argc2-1 ? "\n" : " "));
                    }
                    exit(0);
                }
            }
            
            // Execute external command
            execvp(argv[0], argv);
            fprintf(stderr, "Command not found!\n");
            exit(1);
            
        } else if (pid > 0) {
            // Parent process
            pids[i] = pid;
            
            // Set process group - all processes in pipeline share same group
            if (i == 0) {
                pgid = pid;
                setpgid(pid, pgid);
                if (!bg) {
                    fg_pid = pgid;  // Track the process group for foreground jobs
                }
            } else {
                setpgid(pid, pgid);  // Join the group of the first process
            }
            
            // Close pipe file descriptors in parent
            if (i > 0) close(last_in);
            if (i < npipes - 1) {
                close(pipefds[i%2][1]);
                last_in = pipefds[i%2][0];
            }
            
            // Add background job to job list (only for the first process in pipeline)
            if (i == 0 && bg) {
                add_job(pgid, cmd, JOB_RUNNING);  // Use original command, not pipes[0]
                printf("[%d] %d\n", get_next_job_num()-1, (int)pgid);
                fflush(stdout);  // Add flush for background job notification
            }
        } else {
            // Fork failed
            perror("fork");
            return;
        }
    }
    
    // Wait for foreground processes to complete
    if (!bg) {
        int status;
        for (int i = 0; i < npipes; ++i) {
            waitpid(pids[i], &status, WUNTRACED);
            
            // If any process was stopped, the whole pipeline is considered stopped
            if (WIFSTOPPED(status)) {
                // Process was stopped by Ctrl+Z
                // The signal handler will add it to the job list and print message
                return;  // Don't reset fg_pid, signal handler will do it
            }
        }
        
        // All processes completed normally, reset foreground tracking
        fg_pid = 0;
        fg_cmd[0] = '\0';
    }
}

static void check_bg_jobs_and_notify(void) {
    job_t *j = job_list;
    job_t *prev = NULL;
    
    while (j) {
        int status = 0;
        pid_t res = waitpid(j->pid, &status, WNOHANG);
        if (res > 0) {
            // Check if process exited normally (regardless of exit code)
            if (WIFEXITED(status)) {
                printf("%s with pid %d exited normally\n", j->command, (int)j->pid);
                fflush(stdout);
            } else if (WIFSIGNALED(status)) {
                printf("%s with pid %d exited abnormally\n", j->command, (int)j->pid);
                fflush(stdout);
            }
            
            job_t *to_remove = j;
            if (prev) {
                prev->next = j->next;
            } else {
                job_list = j->next;
            }
            j = j->next;
            free(to_remove);
        } else {
            prev = j;
            j = j->next;
        }
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    char cwd[4096];
    getcwd(cwd, sizeof(cwd));
    homeDir = strdup(cwd);
    prevDir = NULL;
    
    struct passwd *pw = getpwuid(getuid());
    
    // Set up signal handlers with sigaction
    struct sigaction sa_int, sa_tstp;
    
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;  // Restart interrupted system calls
    
    sa_tstp.sa_handler = handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;  // Restart interrupted system calls
    
    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGTSTP, &sa_tstp, NULL);
    
    while (1) {
        check_bg_jobs_and_notify();
        print_prompt(pw->pw_name, hostname, homeDir);
        
        char *line = read_line();
        if (!line) {
            printf("logout\n");
            fflush(stdout);
            kill_all_children();
            exit(0);
        }
        
        if (strlen(line) == 0) { 
            free(line); 
            continue; 
        }
        
        if (!is_valid_command(line)) {
            printf("Invalid Syntax!\n");
            free(line); 
            continue;
        }
        
        char *cmds[64];
        int ncmds = split_commands(line, cmds, ";");
        
        for (int ci = 0; ci < ncmds; ++ci) {
            int bg = is_background(cmds[ci]);
            
            char *cmd = strdup(cmds[ci]);
            char *saveptr = NULL;
            char *argv[128];
            int argc = 0;
            char *tok = strtok_r(cmd, " \t", &saveptr);
            while (tok && argc < 127) {
                argv[argc++] = tok;
                tok = strtok_r(NULL, " \t", &saveptr);
            }
            argv[argc] = NULL;
// ############## LLM Generated Code Ends ##############
            int handled = 0;
            if (argc > 0) {
                if (strcmp(argv[0], "hop") == 0) {
                    builtin_hop(argc, argv, homeDir, &prevDir);
                    handled = 1;
                } else if (strcmp(argv[0], "reveal") == 0) {
                    builtin_reveal(argc, argv, homeDir, &prevDir);
                    handled = 1;
                } else if (strcmp(argv[0], "log") == 0) {
                    builtin_log(argc, argv, logfile, 1, NULL);
                    handled = 1;
                } else if (strcmp(argv[0], "activities") == 0) {
                    builtin_activities();
                    handled = 1;
                } else if (strcmp(argv[0], "ping") == 0) {
                    builtin_ping(argc, argv);
                    handled = 1;
                } else if (strcmp(argv[0], "fg") == 0) {
                    builtin_fg(argc, argv);
                    handled = 1;
                } else if (strcmp(argv[0], "bg") == 0) {
                    builtin_bg(argc, argv);
                    handled = 1;
                }
            }
            
            if (!handled) {
                log_store_only(logfile, line);
                execute_command(cmds[ci], bg, logfile);
            }
            
            free(cmd);
        }
        
        free(line);
    }
    
    free(homeDir);
    if (prevDir) free(prevDir);
    return 0;
}