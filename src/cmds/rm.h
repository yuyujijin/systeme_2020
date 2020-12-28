#ifndef RM_H
#define RM_H
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
//#include "../tar.h"
#include "../tar_manipulation.h"
#include "../useful.h"

int main(int argc, const char** argv);

int rm_call(int argc,const char** argv);

int rm_tar_option(const char *argv, int start);

int last_is_tar(const char* argv);

int rm_tar(const char *argv, int tar_index);

#endif
