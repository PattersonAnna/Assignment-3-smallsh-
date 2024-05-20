#ifndef SMALLSH_H
#define SMALLSH_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

/*
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>
*/

void start();
void exitProgram();
void getCD(char *path);
void getStatus();
void executingOtherCommands(char *userInput);
#endif
