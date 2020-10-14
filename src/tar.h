#ifndef TAR_H
#define TAR_H
#include <string.h>
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* tar Header Block, from POSIX 1003.1-1990.  */

#define BLOCKSIZE 512
#define BLOCKBITS 9

/* POSIX header.  */

/* Note that sizeof(struct posix_header) == BLOCKSIZE */

struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
  char junk[12];                /* 500 */
};                              /* Total: 512 */

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* ... */

#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */

/* ... */


/* Compute and write the checksum of a header, by adding all
   (unsigned) bytes in it (while hd->chksum is initially all ' ').
   Then hd->chksum is set to contain the octal encoding of this
   sum (on 6 bytes), followed by '\0' and ' '.
*/

void set_checksum(struct posix_header *hd) {
  memset(hd->chksum,' ',8);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  sprintf(hd->chksum,"%06o",sum);
}

/* Check that the checksum of a header is correct */

int check_checksum(struct posix_header *hd) {
  unsigned int checksum;
  sscanf(hd->chksum,"%o ", &checksum);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  for (int i=0;i<8;i++) { sum += ' ' - hd->chksum[i]; }
  return (checksum == sum);
}


/*
  Read all the information from the tarballs
  and return a struct posix header containing them
*/
struct posix_header* posix_header_from_tarFile(const char *path){
  int f=open(path,O_RDONLY);
  struct posix_header *p=malloc(sizeof(struct posix_header));
  if(f<0){
    perror(path);
    return NULL;
  }
  if(read(f,p->name,100)<0)return NULL;;
  if(read(f,p->mode,8)<0)return NULL;
  if(read(f,p->uid,8)<0)return NULL;
  if(read(f,p->gid,8)<0)return NULL;
  char size[12];
  if(read(f,size,12)<0)return NULL;
  unsigned int filesize= strtol(size,NULL,8);
  sprintf(p->size,"%011o",filesize);
  if(read(f,p->mtime,12)<0)return NULL;
  if(read(f,p->chksum,8)<0)return NULL;
  if(read(f,&p->typeflag,1)<0)return NULL;
  if(read(f,p->linkname,100)<0)return NULL;
  if(read(f,p->magic,6)<0)return NULL;
  if(read(f,p->version,2)<0)return NULL;
  if(read(f,p->uname,32)<0)return NULL;
  if(read(f,p->gname,32)<0)return NULL;
  if(read(f,p->devmajor,8)<0)return NULL;
  if(read(f,p->devminor,8)<0)return NULL;
  if(read(f,p->prefix,155)<0)return NULL;
  if(read(f,p->junk,12)<0)return NULL;
  return p;
}
#endif
