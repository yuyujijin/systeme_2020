#ifndef RMDIR_H
#define RMDIR_H
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
//#include "../tar.h"
#include "../tar_manipulation.h"

int last_is_tar(char* argv);

int main(int argc, char** argv);

//call rmdir id we're in a regular path
//else calls rmdir path
int rmdir_call(int argc, char** argv);

int rmdir_tar(char *argv);

#endif
