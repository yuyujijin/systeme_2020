#ifndef CAT_H
#define CAT_H
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include<sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "../tar.h"
#include "../tar_manipulation.h"
#include "../useful.h"
int cat(char *const argv[],int argc);//this is where we decide for each args to execute it normally or as a tar, we call cat_tar
int cat_tar(char *const arg);//this is used to handle tar file
#endif
