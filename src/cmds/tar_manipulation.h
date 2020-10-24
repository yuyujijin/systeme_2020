#ifndef TAR_MANIPULATION_H
#define TAR_MANIPULATION_H
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "tar.h"

/* check for every header of a tar file if its checksum is correct */
int isTar(char*);

/* returns the offset of the first empty block */
size_t offsetTar(char *path);

/* reads from stdin and adds it to a tar */
int addTar(char *path, char name[100],  char typeflag);

/* removes file with corresponding name */
int rmTar(char *path, char *name);

int isEmpty(struct posix_header*);

int is_source(const char* path);

/* tells if filename is in tarball tarname */
int exists(char *tarname, char *filename);

#endif
