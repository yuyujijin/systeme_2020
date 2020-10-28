#include <stdio.h>
#include "cd.h"

#define BUF_SIZE 512

int cp_2args(char**);
char* getLastArg(char *path);

int main(int argc, char**argv){
  getLastArg(argv[1]);
  if(argc < 3) return -1;
  if(argc == 3) return cp_2args(argv + 1);

}

int cp_2args(char **argv){
  /* we'll use a fork for this :
  son we'll be the reader of the copied file, father will be the writer */
  /* we first begin by saving actual cwd and TARPATH, so we can reset it back after everything is done */
  char *pwd = getcwd(NULL,0);
  char *tarpath = getenv("TARPATH");

  int fd_pipe[2];
  pipe(fd_pipe);

  char rd_buf[BUF_SIZE];
  memset(rd_buf,'\0',BUF_SIZE);
  char *elem;
  int fd, size;

  switch(fork()){
    case -1: return -1;
    /* son */
    case 0: close(fd_pipe[0]);
    elem = getLastArg(argv[0]);
    /* if path argv[0] is more than just elem, we try to access the path */
    if(strlen(argv[0]) - strlen(elem) > 0){
      if(cd(argv[0] - strlen(elem) - 1) < 0) return -1;
    }
    /* case 1 : we're in a tar */
    if(strcmp(getenv("TARPATH"),"") != 0){
      return 0;
    }
    /* case 2 : we're not in a tar */
    fd = open(elem,O_RDONLY);
    if(fd < 0) return -1;

    while((size = read(fd,rd_buf,BUF_SIZE)) > 0){
      write(fd_pipe[1], rd_buf, size);
    }
    close(fd);
    close(fd_pipe[1]);

    exit(0);
    break;
    default: close(fd_pipe[1]);
    elem = getLastArg(argv[1]);
    /* if path argv[1] is more than just elem, we try to access the path */
    if(strlen(argv[1]) - strlen(elem) > 0){
      if(cd(argv[1] - strlen(elem) - 1) < 0) return -1;
    }

    /* case 1 : we're in a tar */
    if(strcmp(getenv("TARPATH"),"") != 0){
      return 0;
    }
    /* case 2 : we're not in a tar */
    fd = open(elem,O_WRONLY | O_CREAT);
    if(fd < 0) return -1;

    while((size = read(fd_pipe[0],rd_buf,BUF_SIZE)) > 0){
      write(fd, rd_buf, size);
    }

    close(fd);
    close(fd_pipe[0]);
    break;
  }

  return 1;
}

char* getLastArg(char *path){
  char *s = strtok(path, "/");
  char *temp = s;
  do{
    temp = strtok(NULL,"/");
  }while(temp != NULL);

  return s;
}
