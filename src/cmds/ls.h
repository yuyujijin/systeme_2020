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
#include <math.h>
#include <string.h>
#include "../tar.h"
#include "../tar_manipulation.h"
int ls(char *const args[],int argc);//we call this function in shell
int has_option(char *const args[],int argc);//return -1 if no option, if option is -l then return index of the option in args[]
int ls_tar(const char *args,int option);//ls for all tar file
int maxNbDigit(struct posix_header** posix_header);//return the max nb of digit of all file size
void convert_stmode(struct posix_header* posix_header,char mode[]);//convert the mode found in posix_header into a readable mode
#endif
