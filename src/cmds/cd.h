#ifndef CD_H
#define CD_H

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "../useful.h"
#include "../tar_manipulation.h"



/*
path_simplifier will simplify the specified path (removing '..', '.' and '/' when possible)
and return a struct containing the new path, the tarball's path and the tarball's name

it works using a LIFO structure (a stack), and adding every words from path in it, and creating a struct special_path from it
*/
/* get_full_path concatenates tar_path + "/" + path */
char* get_full_path(char *path, char *tar_path);
struct special_path special_path_maker(char *path);
int cd(char *path);

#endif
