#ifndef LS_H
#define LS_H
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
#include <string.h>
#include <math.h>
#include "tar.h"
#include "tar_manipulation.h"
int ls(char *const args[],int argc);
int has_option(char *const args[],int argc);
int has_option(char *const args[],int argc);
int ls_tar(const char *args,int option);
int ls_tar_option(struct posix_header* posix_header);
#endif
