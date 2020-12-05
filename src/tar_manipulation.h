#ifndef TAR_MANIPULATION_H
#define TAR_MANIPULATION_H
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "tar.h"


/* check for every header of a tar file if its checksum is correct */
int isTar(char*);

/* returns the offset of the first empty block */
size_t offsetTar(char *path);

/* reads from stdin and adds it to a tar */
int addTar(char *path, char name[100]/*,  char typeflag*/);

/* removes file with corresponding name */
int rmTar(char *path, char *name);

int isEmpty(struct posix_header*);

/*
  Verify if any of the path in args is a tar, if it is, it adds its position in args to an int[]
  We return this int[] so that we can use it later
*/

void has_Tar(char *const args[],int argc,int *tarIndex);// If args[0] has a tar in it's arguments then tarIndex[0]==1

//check if argv has a tar (not a dir named xx.tar) and returns index after
//xx.tar/
int has_tar(const char* argv);


/*
  Let's say our path is "a/b/c.tar/d/e"this methods returns "a/b/c.tar"
*/
char* get_tar_from_full_path(const char * path);

/*
  Returns an array of posix_header of all the file(directory too) inside path,
  if we have a tar arborescence like : a.tar/b a.tar/c a.tar/d/e
  posix_header_from_tarFile("a.tar") -> {b , c ,d/ }(array of posix_header)
  posix_header_from_tarFile("a.tar/d/") -> {e}
  if the path inside the tar doesn't exit or any other error it returns NULL
*/
struct posix_header** posix_header_from_tarFile(const char *path);

/*
  A methods that simply return 1 if path="a.tar" or path="a.tar/" and return 0 otherwise
*/
int is_source(const char* path);

/*
  Return the data of the file named path,
  If path="a.tar/c" then return the data inside c,
  If the file doesn't exist or path leads to a folder returns NULL
*/
char * data_from_tarFile(const char *path);

int exists(char* tarname, char* filename);

char *substr(const char *src,int start,int end);

int file_exists_in_tar(char* path, char* name);

#endif
