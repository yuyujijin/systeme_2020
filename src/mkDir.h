#ifndef MKDIR_h
#define MKDIR_h

#include <stdlib.h>
#include <stdio.h>
#include<limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int splitMkDir(const char* argv);

char *substr(const char *src,int start,int end);

int nextSpace(const char *path,int index);

int mkDirectory(const char* argv);

#endif

