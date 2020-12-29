#include <unistd.h>
#include "tar.h"
#include "tar_manipulation.h"
int isCharEmpty(char buf[BLOCKSIZE]){
  for(int i = 0; i < BLOCKSIZE; i++)
    if(buf[i] != '\0') return 0;
  return 1;
}
int main(int argc, char **argv){
  int fd = open(argv[1], O_RDONLY);
  char h[BLOCKSIZE];
  memset(h,0,BLOCKSIZE);
  int i = 0;
  size_t filesize = lseek(fd,0,SEEK_END);
  lseek(fd,0,SEEK_SET);
  while(1){
    if(filesize / (i * BLOCKSIZE) <= 0) break;
    read(fd,h,BLOCKSIZE);
    printf("bloc n°%d :\n%s\n",i++,h);
    memset(h,0,BLOCKSIZE);
  }
  return 0;
}
