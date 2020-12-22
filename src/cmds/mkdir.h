
#ifndef MKDIR_H
#define MKDIR_H
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
//#include "../tar.h"
#include "../useful.h"
#include "../tar_manipulation.h"

int addDirTar(char* path, char* name);

#endif
