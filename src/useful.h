#ifndef USEFUL_H
#define USEFUL_H

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "cmds/cd.h"

char* path_simplifier(char* path);
char* pathminus(char *path, char *lastarg);
char *getLastArg(char *path);
int cdTo(char *path, char* last_arg);

#endif
