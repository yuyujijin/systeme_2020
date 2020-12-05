#ifndef RMDIR_H
#define RMDIR_H 
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
//#include "../tar.h"
#include "../tar_manipulation.h"

int rmdir_call(int argc,const char** argv);

int rmdir_tar(const char *argv, int tar_index);

#endif
