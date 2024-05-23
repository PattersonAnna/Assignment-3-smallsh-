#ifndef SMALLSH_H
#define SMALLSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>

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
#endif
