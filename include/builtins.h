#ifndef BUILTINS_H
#define BUILTINS_H

#define LOG_MAX 15

void builtin_hop(int argc, char **argv, const char *homeDir, char **prevDir);
void builtin_reveal(int argc, char **argv, const char *homeDir, char **prevDir);
int builtin_log(int argc, char **argv, const char *logfile, int skip_store, const char *last_cmd);
int log_store_only(const char *logfile, const char *cmd);

#endif