#ifndef MV_H
#define MV_H
#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../useful.h"

/*
  function to simulate mv, we first cp then rm all file except destination
*/
int mv(int argc, char **argv);
#endif
