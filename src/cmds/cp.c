#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include "cd.h"

#define BUF_SIZE 512

/* copy arg[0] into path arg[1] */
int cp_2args(char**);
/* same as cp_2args but works with directories */
int cp_r2args(char **);
/* copy every args from argv[0] to argv[argc - 2] into argv[argc - 1] */
int cp_nargs(char **argv, int argc);
/* same as cp_nargs but works with directories */
int cp_rnargs(char **argv, int argc)
/* returns the last arg of a path */
char* getLastArg(char *path);

char* getTPTarName();
char* getTPPath(char *);

int main(int argc, char**argv){
  if(argc < 3) return -1;
  if(strcmp(argv[1],"-r" == 0){
    if(argc == 4) return cp_r2args(argv + 2);
    return cp_rnargs(argc + 2, argc - 2);
  }
  if(argc == 3) return cp_2args(argv + 1);
  return cp_nargs(argv + 1, argc - 1);
}

int cp_nargs(char **argv, int argc){
  for(int i = 0; i < argc - 1; i++){
    char *cp[2];

    char cp_1[strlen(argv[i]) + 1];
    strcpy(cp_1,argv[i]);
    strcat(cp_1,"\0");
    cp[0] = cp_1;

    char cp_2[strlen(argv[argc - 1]) + 1];
    strcpy(cp_2,argv[argc - 1]);
    strcat(cp_2,"\0");
    cp[1] = cp_2;

    if(cp_2args(cp) < 0){ perror("cp"); return -1; }
  }

  return 1;
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
      char* tarname = getTPTarName();
      char* tpath = getTPPath(elem);
      int old_stdin = dup(STDIN_FILENO);
      dup2(fd_pipe[1], STDIN_FILENO);

      struct posix_header* hd = getHeader(tarname,tpath);
      if(hd != NULL && hd->typeflag == '5'){ perror("cp"); return -1; }
      if(rdTar(tarname,tpath) < 0){ perror("cp"); return -1; }

      free(tarname);
      free(tpath);

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

    default:
    close(fd_pipe[1]);

    elem = getLastArg(argv[1]);

    /* if path argv[1] is more than just elem, we try to access the path */
    if(strchr(argv[1],'/') != NULL){
      if(argv[1][strlen(argv[1]) - 1] != '/' && strlen(argv[1]) > strlen(elem) + 1) argv[1][strlen(argv[1]) - strlen(elem)] = '\0';
      if(cd(argv[1]) < 0){ perror("cp"); return -1; }
    }

    /* in case arg[1] is a dir, file is copied with its original name */
    if(elem[strlen(elem) - 1] == '/') elem = getLastArg(argv[0]);

    /* case 1 : we're in a tar */
    if(strcmp(getenv("TARPATH"),"") != 0){
      /* redirect the pipe out in stdin and addTar */
      int old_stdout = dup(STDOUT_FILENO);
      dup2(fd_pipe[0], STDOUT_FILENO);

      char* tarname = getTPTarName();
      char* tpath = getTPPath(elem);


      if(addTar(tarname,tpath) < 0){ perror("cp"); return -1; }

      /* reset */
      dup2(old_stdout,STDOUT_FILENO);
      close(fd_pipe[0]);

      free(tarname);
      free(tpath);

      chdir(pwd);
      setenv("TARPATH",tarpath,1);

      return 1;
    }
    /* case 2 : we're not in a tar */
    fd = open(elem,O_WRONLY | O_CREAT,"00777");
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

int cp_rnargs(char **argv, int argc){

}

int cp_r2args(char **argv){
  
}

char* getTPTarName(){
  char *tpath = malloc(sizeof(char) * strlen(getenv("TARPATH")));
  strcat(tpath, getenv("TARPATH"));

  return strtok(tpath,"/");
}

char* getTPPath(char* elem){
  char *tpath = malloc(sizeof(char) * strlen(getenv("TARPATH")));
  strcat(tpath, getenv("TARPATH"));

  char *firsttoken = strtok(tpath,"/");

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

  free(temp);

  if(path[strlen(path) - 1] == '/'){
    char *new_s = malloc((strlen(s) + 2) * sizeof(char));
    strcpy(new_s,s);
    strcat(new_s,"/");
    strcat(new_s,"\0");
    s = new_s;
  }

  return s;
}
