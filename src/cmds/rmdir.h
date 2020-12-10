#ifndef MKDIR_H
#define MKDIR_H
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
//#include "../tar.h"
#include "../tar_manipulation.h"

int last_is_tar(const char* argv);

int main(int argc, const char** argv);

//call rmdir id we're in a regular path
//else calls rmdir path
int rmdir_call(int argc,const char** argv);

int last_is_tar(const char* argv);

int rmdir_tar(const char *argv, int tar_index);

#endif
