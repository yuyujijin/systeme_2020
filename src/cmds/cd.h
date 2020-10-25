#ifndef CD_H
#define CD_H

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../tar_manipulation.h"
#include <errno.h>

int cd(char *path);

int cd_aux(char *path);

#endif
