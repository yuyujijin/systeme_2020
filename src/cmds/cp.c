#include <stdio.h>
#include "cd.h"

#define BUF_SIZE 512

int cp_2args(char**);
char* getLastArg(char *path);
const char* getTPTarName();
const char* getTPPath(char *);

int main(int argc, char**argv){
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
    if(strlen(argv[0]) > strlen(elem)){
      argv[0][strlen(argv[0]) - strlen(elem)] = '\0';
      if(cd(argv[0]) < 0){ perror("cp"); return -1; }
    }
    /* case 1 : we're in a tar */
    if(strcmp(getenv("TARPATH"),"") != 0){
      /* redirect stdout in the pipe entry and use rdTar */
      int old_stdin = dup(STDIN_FILENO);
      dup2(fd_pipe[1], STDIN_FILENO);

      const char* tarname = getTPTarName();
      const char* tpath = getTPPath(elem);

      if(rdTar(tarname,tpath) < 0){ perror("cp"); return -1; }

      close(fd_pipe[1]);
      /* reset */
      dup2(old_stdin, STDIN_FILENO);
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
    if(strlen(argv[1]) > strlen(elem)){
      argv[1][strlen(argv[1]) - strlen(elem)] = '\0';
      if(cd(argv[1]) < 0){ perror("cp"); return -1; }
    }

    /* case 1 : we're in a tar */
    if(strcmp(getenv("TARPATH"),"") != 0){
      // TODO...
      /* redirect the pipe out in stdin and addTar */
      int old_stdout = dup(STDOUT_FILENO);
      dup2(fd_pipe[0], STDOUT_FILENO);

      const char* tarname = getTPTarName();
      const char* tpath = getTPPath(elem);

      if(addTar(tarname,tpath) < 0){ perror("cp"); return -1; }

      /* reset */
      dup2(old_stdout,STDOUT_FILENO);
      close(fd_pipe[0]);
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

const char* getTPTarName(){
  char *tpath = malloc(sizeof(char) * strlen(getenv("TARPATH")));
  strcat(tpath, getenv("TARPATH"));

  return strtok(tpath,"/");
}

const char* getTPPath(char* elem){
  char *tpath = malloc(sizeof(char) * strlen(getenv("TARPATH")));
  strcat(tpath, getenv("TARPATH"));

  const char *firsttoken = strtok(tpath,"/");

  if(strlen(getenv("TARPATH")) == strlen(firsttoken)) return elem;

  char *path = malloc(sizeof(char) * (strlen(getenv("TARPATH")) - strlen(firsttoken) + strlen(elem)));
  strcat(path, getenv("TARPATH") + 1 + strlen(firsttoken));
  strcat(path,"/");
  strcat(path,elem);
  strcat(path,"\0");

  return path;
}

char* getLastArg(char *path){
  char *path_cpy = malloc(sizeof(char) * strlen(path));
  strcat(path_cpy,path);

  char *s = strtok(path_cpy, "/");
  char *temp = s;
  do{
    s = temp;
    temp = strtok(NULL,"/");
  }while(temp != NULL);

  return s;
}
