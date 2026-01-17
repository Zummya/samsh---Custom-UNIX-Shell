#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED
} job_state_t;

typedef struct job {
    int job_num;
    pid_t pid;
    char command[4096];
    job_state_t state;
    struct job *next;
} job_t;

extern volatile pid_t fg_pid;
extern char fg_cmd[4096];
extern job_t *job_list;

void add_job(pid_t pid, const char *cmd, job_state_t state);
pid_t get_job_pid(int idx);
int get_next_job_num(void);
job_t *find_job_by_num(int job_num);
char *get_job_cmd(int idx);
void remove_job(int idx);
void builtin_jobs(void);
void builtin_activities(void);
void builtin_ping(int argc, char **argv);
void builtin_fg(int argc, char **argv);
void builtin_bg(int argc, char **argv);
// void check_bg_jobs(void);

#endif