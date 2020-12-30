#ifndef UNIT_H
#define UNIT_H
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "cmds/cd.h"
void cd_uni_test();
void ls_uni_test();
void cp_uni_test();
void cat_uni_test();
void rmdir_uni_test();
void mkdir_uni_test();
void rm_uni_test();
void mv_uni_test();
//create all the file necessary inside the folder test
void createTestFolder();
#endif
