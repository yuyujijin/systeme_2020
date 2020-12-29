#ifndef MV_H
#define MV_H
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
  function to simulate mv, we first cp then rm all file except destination
*/
int mv(int argc, char const *argv[]);
#endif
