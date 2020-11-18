#ifndef CD_H
#define CD_H

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tar_manipulation.h"

typedef struct special_path{
  char *path;
  char *tar_path;
  char *tar_name;
} special_path;

/*
path_simplifier will simplify the specified path (removing '..', '.' and '/' when possible)
and return a struct containing the new path, the tarball's path and the tarball's name

it works using a LIFO structure (a stack), and adding every words from path in it, and creating a struct special_path from it
*/
struct special_path path_simplifier(char* path);
/* get_full_path concatenates tar_path + "/" + path */
char* get_full_path(char *path, char *tar_path);

int cd(char *path);

#endif
