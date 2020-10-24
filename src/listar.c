#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include "tar.h"

int isEmpty(struct posix_header* p){
  if((p->name)[0] == '\0') return 1;
  return 0;
}

int main(int argc, char **argv){
  if(argc != 2) return 1;

  struct posix_header tampon;
  int fd;

  fd = open(argv[1],O_RDONLY);

  while(1){
    size_t size = read(fd, &tampon, sizeof(struct posix_header));

    if(isEmpty(&tampon)) break;

    int filesize;
    sscanf(tampon.size,"%o", &filesize);

    printf("name: %s\n", tampon.name);
    printf("mode: %d\n", tampon.mode);
    printf("size: %d\n", tampon.size);
    printf("chksum: %d\n", tampon.chksum);
    printf("typeflag: %d\n", tampon.typeflag);
    printf("magic: %d\n", tampon.magic);

    int s = (filesize + 512 - 1)/512;

    struct posix_header* temp = malloc(sizeof(struct posix_header) * s);
    read(fd, temp, s * BLOCKSIZE);
    free(temp);
  }

  printf("Le fichier n'existe pas dans l'archive...\n");

  close(fd);
  return 0;
}
