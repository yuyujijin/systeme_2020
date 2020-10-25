#ifndef CD_H
#define CD_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../tar_manipulation.h"

int cd(char *path);

int cd_aux(char *path);

#endif
