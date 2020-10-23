#ifndef MKDIR_H
#define MKDIR_H
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>

int mkdir_tar(const char* argv);
int has_tar(const char* argv);
int addDirTar(char* path, char* name);

#endif
