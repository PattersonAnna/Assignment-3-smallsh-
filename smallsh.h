#ifndef SMALLSH_H
#define SMALLSH_H

#include <stdio.h>      // For standard I/O functions (printf, fgets, etc.)
#include <stdlib.h>     // For standard library functions (exit, malloc, etc.)
#include <string.h>     // For string manipulation functions (strtok, strcpy, etc.)
#include <dirent.h>     // For directory handling functions (opendir, readdir, etc.)
#include <unistd.h>     // For POSIX API functions (fork, execvp, getpid, etc.)
#include <pwd.h>        // For password file database access (getpwuid, etc.)
#include <sys/types.h>  // For data types used in system calls (pid_t, etc.)
#include <sys/wait.h>   // For waitpid and associated macros
#include <stdbool.h>    // For boolean type and values (true, false)
#include <fcntl.h>      // For file control options (open, O_RDONLY, etc.)
#include <sys/stat.h>   // For file status and permissions (stat, S_ISREG, etc.)
#include <errno.h>      // For error handling (errno) 
#include <signal.h> // Include the signal header

struct Command{
    char *command;
    char *arg[2049];
    char *input;
    char *output;
    bool background;  
};

void start();
void exitProgram();
void getCD(char *path);
void getStatus();
void executingOtherCommands(struct Command *current);
void slitCommand(char *userInput);
void printCommand(struct Command *current);
void freeCommand(struct Command *commands);
int numArgs(struct Command *current);
void background(struct Command *current);
void handle_sigchld(int sig);
void checkBackground(struct Command *current);
void ignoreCtrlC(int signum);
void ignore_pkill(int signal);
#endif
