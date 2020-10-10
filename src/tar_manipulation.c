#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include "tar.h"

/* check for every header of a tar file if its checksum is correct */
int isTar(char*);

/* reads from stdin and adds it to a tar */
int addTar(char *path, char name[100],  char typeflag);

int isEmpty(struct posix_header*);

int main(int argc, char** argv){
  char name[100];
  memset(name,'\0',100);
  sprintf(name,"nom");

  int s = addTar("test.tar",name,0);
  printf("%d\n",s);
  return 0;
}

int addTar(char *path, char name[100], char typeflag){
  int fd;

  fd = open(path,O_WRONLY);
  if(fd < 0) return -1;

  if(!isTar(path)) return -1;

  /* We use a bufer of size 512 bytes, that reads in STDIN while it can */
  char *buffer = malloc(sizeof(char) * 512);
  if(buffer == NULL) return -1;
  size_t bufsize = 0;
  /* Put it at '\0' on every bytes, in case we didnt read 512 bytes */
  memset(buffer,'\0',512);
  size_t size;
  /* read everything from STDIN and store in the buffer */
  while((size = read(1, buffer + bufsize, 512)) > 0){
    bufsize += size;
    buffer = realloc(buffer, bufsize + 512);
    memset(buffer + bufsize, '\0', 512);
  }

  /* We must put the reading head just before the 2 empty bloc at the end of the tar */
  size_t offt = lseek(fd, -(512 * 2), SEEK_END);

  /* Now we write the header */
  struct posix_header hd;set_checksum(&hd);
  memset(&hd,'\0',sizeof(struct posix_header));

  memcpy(hd.name, name, strlen(name));
  sprintf(hd.mode,"0000700");

  sprintf(hd.size, "%011o", bufsize);

  hd.typeflag = typeflag;
  memcpy(hd.magic,"ustar",5);
  memcpy(hd.version,"00",2);
  set_checksum(&hd);

  if(check_checksum(&hd) < 0) return -1;

  if(write(fd, &hd, sizeof(struct posix_header)) < 0) return -1;;

  /* And then we write the blocs we've red before */
  for(int i = 0; i < (bufsize + 512 - 1)/512; i++){
    if(write(fd, buffer + (i * 512), 512) < 0) return -1;
  }

  /* We put the two empty blocks at the end of the tar */
  char emptybuf[512];
  memset(emptybuf,'\0',512);
  for(int i = 0; i < 2; i++){ if(write(fd, emptybuf,512) < 0) return -1; }

  close(fd);

  return 0;
}

int isEmpty(struct posix_header* p){
  if((p->name)[0] == '\0') return 1;
  return 0;
}

int isTar(char* path){
  struct posix_header tampon;
  int fd;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0) return 0;

  while(1){
    /* create the buffer to read the header */
    size_t size = read(fd, &tampon, sizeof(struct posix_header));

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    /* if checksum fails, then its not a proper tar */
    if(check_checksum(&tampon) == 0) return 0;

    /* we get the size of the file for this header */
    int filesize;
    sscanf(tampon.size,"%o", &filesize);

    printf("%s\n",tampon.name);

    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }

  close(fd);

  return 1;
}
