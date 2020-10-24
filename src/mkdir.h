#ifndef MKDIR_H
#define MKDIR_H
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>

int mkdir_tar(const char* argv,int start);
int has_tar(const char* argv);
int addDirTar(char* path, char* name);
char *substr(const char *src,int start,int end);
int file_exists_in_tar(char* path, char* name);

#endif
