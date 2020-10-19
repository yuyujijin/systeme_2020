#ifndef CD_H
#define CD_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int cd(char *path);

int cd_aux(char *path);

#endif
