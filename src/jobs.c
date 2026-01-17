#include "jobs.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

job_t *job_list = NULL;
static int next_job_num = 1; // Add this to track unique job numbers
// ############## LLM Generated Code Begins ##############
void add_job(pid_t pid, const char *cmd, job_state_t state)
{
    job_t *new_job = malloc(sizeof(job_t));
    new_job->job_num = next_job_num++; // Assign unique job number
    new_job->pid = pid;
    strncpy(new_job->command, cmd, sizeof(new_job->command) - 1);
    new_job->command[sizeof(new_job->command) - 1] = '\0';
    new_job->state = state;
    new_job->next = job_list;
    job_list = new_job;
}
// ############## LLM Generated Code Ends ##############
pid_t get_job_pid(int idx)
{
    job_t *j = job_list;
    int i = 0;
    while (j)
    {
        if (i == idx)
            return j->pid;
        j = j->next;
        i++;
    }
    return 0;
}

int get_next_job_num(void)
{
    return next_job_num; // Return the next job number directly
}

char *get_job_cmd(int idx)
{
    job_t *j = job_list;
    int i = 0;
    while (j)
    {
        if (i == idx)
            return j->command;
        j = j->next;
        i++;
    }
    return "";
}

void remove_job(int idx)
{
    job_t **pj = &job_list;
    int i = 0;
    while (*pj)
    {
        if (i == idx)
        {
            job_t *to_free = *pj;
            *pj = to_free->next;
            free(to_free);
            return;
        }
        pj = &(*pj)->next;
        i++;
    }
}

// Add this function to find a job by its job number
job_t *find_job_by_num(int job_num)
{
    for (job_t *j = job_list; j; j = j->next)
        if (j->job_num == job_num)
            return j;
    return NULL;
}

void builtin_jobs(void)
{
    job_t *curr = job_list;
    while (curr)
    {
        printf("[%d] %d %s %s\n", curr->job_num, curr->pid, curr->command,
               curr->state == JOB_RUNNING ? "[Running]" : "[Stopped]");
        curr = curr->next;
    }
}
// ############## LLM Generated Code Begins ##############
void builtin_activities(void)
{
    job_t *arr[64];
    int count = 0;

    // Collect all jobs
    job_t *j = job_list;
    while (j && count < 64)
    {
        arr[count++] = j;
        j = j->next;
    }

    // Sort lexicographically by command name
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (strcmp(arr[i]->command, arr[j]->command) > 0)
            {
                job_t *tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }

    // Print sorted jobs
    for (int i = 0; i < count; i++)
    {
        printf("[%d] : %s - %s\n", arr[i]->pid, arr[i]->command,
               arr[i]->state == JOB_RUNNING ? "Running" : "Stopped");
    }
}
// ############## LLM Generated Code Ends ##############
void builtin_ping(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Invalid syntax!\n");
        return;
    }
    char *endptr;
    long signal_num = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0')
    {
        fprintf(stderr, "Invalid syntax!\n");
        return;
    }

    pid_t pid = (pid_t)strtol(argv[1], &endptr, 10);
    if (*endptr != '\0')
    {
        fprintf(stderr, "Invalid syntax!\n");
        return;
    }

    int actual_signal = signal_num % 32;
    if (kill(pid, actual_signal) == -1)
    {
        fprintf(stderr, "No such process found\n");
    }
    else
    {
        printf("Sent signal %ld to process with pid %d\n", signal_num, (int)pid);
    }
}
// ############## LLM Generated Code Begins ##############
void builtin_fg(int argc, char **argv)
{
    int job_num;
    job_t *job = NULL;

    if (argc == 1)
    {
        // Use most recently created job (first in list)
        if (!job_list)
        {
            fprintf(stderr, "No such job\n");
            return;
        }
        job = job_list; // Most recent job is at the head
    }
    else if (argc == 2)
    {
        job_num = atoi(argv[1]);
        job = find_job_by_num(job_num);
        if (!job)
        {
            fprintf(stderr, "No such job\n");
            return;
        }
    }
    else
    {
        fprintf(stderr, "No such job\n");
        return;
    }

    // Set as foreground process
    extern volatile pid_t fg_pid;
    extern char fg_cmd[4096];

    fg_pid = job->pid;
    strncpy(fg_cmd, job->command, sizeof(fg_cmd) - 1);
    fg_cmd[sizeof(fg_cmd) - 1] = '\0';

    // Print the entire command when bringing it to foreground
    printf("%s\n", job->command);

    // If job is stopped, send SIGCONT to resume it
    if (job->state == JOB_STOPPED)
    {
        kill(job->pid, SIGCONT);
    }

    // Set job state to running
    job->state = JOB_RUNNING;

    // Wait for the job to complete or stop again
    int status;
    waitpid(job->pid, &status, WUNTRACED);

    if (WIFSTOPPED(status))
    {
        // Process was stopped again (by Ctrl+Z)
        job->state = JOB_STOPPED;
        // Don't reset fg_pid here, it's handled in the signal handler
        // The signal handler will print the stopped message and reset fg_pid
    }
    else
    {
        // Process completed (exited or was killed)
        // Remove job from list
        job_t **pj = &job_list;
        while (*pj && *pj != job)
        {
            pj = &(*pj)->next;
        }
        if (*pj)
        {
            *pj = job->next;
            free(job);
        }

        // Reset foreground tracking
        fg_pid = 0;
        fg_cmd[0] = '\0';
    }
}
// ############## LLM Generated Code Ends ##############
void builtin_bg(int argc, char **argv)
{
    int job_num;
    job_t *job = NULL;

    if (argc == 1)
    {
        // Use most recently created job
        if (!job_list)
        {
            fprintf(stderr, "No such job\n");
            return;
        }
        job = job_list; // Most recent job is at the head
    }
    else if (argc == 2)
    {
        job_num = atoi(argv[1]);
        job = find_job_by_num(job_num);
        if (!job)
        {
            fprintf(stderr, "No such job\n");
            return;
        }
    }
    else
    {
        fprintf(stderr, "No such job\n");
        return;
    }

    if (job->state == JOB_RUNNING)
    {
        fprintf(stderr, "Job already running\n");
        return;
    }

    kill(job->pid, SIGCONT);
    job->state = JOB_RUNNING;
    printf("[%d] %s &\n", job->job_num, job->command);
}