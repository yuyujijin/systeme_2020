#include "cd.h"
int cd(char *path){
  if(chdir(path)==-1){
    perror(path);
    return -1;
  }
  return 0;
}
