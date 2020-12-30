#ifndef RM_H
#define RM_H
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include "../tar_manipulation.h"
#include "../useful.h"

int rm_call(int argc, char** argv);
int rm_tar(char *tarname, char *tarpath, int optionR);

#endif
