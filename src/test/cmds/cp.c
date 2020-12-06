#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include "cd.h"

#define BUF_SIZE 512

/* copies argv[0] to argv[1] */
int cp_2args(char**argv);
/* copies every file from {argv[0] ; ... ; argv[argc - 2]} to argv[argc - 1] */
int cp_nargs(char**argv, int argc);
/* copies argv[0] to argv[1], works even if argv[0] is a dir */
int cp_r(char** argv);
/* returns path minus lastarg */
char* pathminus(char *path, char *lastarg);
/* tries to cd to path minus lastarg */
int cdTo(char *path, char* last_arg);
/* reads data in pipe and writes it in file_name. tar specifies if its a tar or not */
int readPipeWriteFile(char *file_name, int tar, int pipe_fd);
/* same as above but reads in file and write in pipe */
int readFileWritePipe(char *file_name, int tar, int pipe_fd);
/* concatenate arg with env variable "TARPATH" */
char *tarpathconcat(char *arg);

int main(int argc, char**argv){
  if(argc < 3) return -1;
  setenv("TARNAME","",1);
  setenv("TARPATH","",1);
  if(strcmp(argv[1],"-r") == 0)
    /* pas assez d'arguments */
    if(argc == 4){ return cp_r(argv + 2); }else{ return -1; }
  if(argc == 3) return cp_2args(argv + 1);
  return cp_nargs(argv + 1, argc - 1);
}

int cp_r(char **argv){
  char *last_arg_1 = (strrchr(argv[0],'/') != NULL)? strrchr(argv[0],'/') : argv[0];
  char *last_arg_2 = (strrchr(argv[1],'/') != NULL)? strrchr(argv[1],'/') : argv[1];

  int pipe_fd[2];
  pipe(pipe_fd);

  switch(fork()){
    case - 1 : return -1;
    case 0 :
    close(pipe_fd[0]);
    if(last_arg_1[0] == '/'){
      if(cdTo(argv[0],last_arg_1) < 0) return -1;
    }
    readFileWritePipe(last_arg_1, strlen(getenv("TARNAME")) > 0, pipe_fd[1]);
    break;
    default :
    close(pipe_fd[1]);
    if(last_arg_2[0] == '/'){
      if(cdTo(argv[1],last_arg_2) < 0) return -1;
    }
    if(strcmp(last_arg_2,"/") == 0) last_arg_2 = last_arg_1;
    readPipeWriteFile(last_arg_2, strlen(getenv("TARNAME")) > 0, pipe_fd[0]);
    break;
  }
  return 0;
}

int cp_2args(char **argv){
  char *last_arg_1 = (strrchr(argv[0],'/') != NULL)? strrchr(argv[0],'/') : argv[0];
  char *last_arg_2 = (strrchr(argv[1],'/') != NULL)? strrchr(argv[1],'/') : argv[1];

  int pipe_fd[2];
  pipe(pipe_fd);

  switch(fork()){
    case - 1 : return -1;
    case 0 :
    close(pipe_fd[0]);
    if(last_arg_1[0] == '/'){
      if(cdTo(argv[0],last_arg_1) < 0) return -1;
    }
    readFileWritePipe(last_arg_1, strlen(getenv("TARNAME")) > 0, pipe_fd[1]);
    break;
    default :
    close(pipe_fd[1]);
    if(last_arg_2[0] == '/'){
      if(cdTo(argv[1],last_arg_2) < 0) return -1;
    }
    if(strcmp(last_arg_2,"/") == 0) last_arg_2 = last_arg_1;
    readPipeWriteFile(last_arg_2, strlen(getenv("TARNAME")) > 0, pipe_fd[0]);
    break;
  }
  return 0;
}

int cp_nargs(char **argv, int argc){
  for(int i = 0; i < argc - 1; i++){
    char *cp[2];
    char cp_1[strlen(argv[i]) + 1];
    char cp_2[strlen(argv[argc - 1]) + 1];

    strcpy(cp_1,argv[i]);
    strcat(cp_1,"\0");
    cp[0] = cp_1;

    strcpy(cp_2,argv[argc - 1]);
    strcat(cp_2,"\0");
    cp[1] = cp_2;

    int w;
    switch(fork()){
      case - 1 : return -1;
      case 0 :
        if(cp_2args(cp) < 0){ perror("cp"); return -1; }
        exit(0);
      default : wait(&w); break;
    }
  }
  return 1;
}

int readPipeWriteFile(char *file_name, int tar, int pipe_fd){
  char buf[BUF_SIZE];
  memset(buf,'\0',BUF_SIZE);

  if(tar){
    char *tpcc = tarpathconcat(file_name);

    struct posix_header* hd = getHeader(getenv("TARNAME"),tpcc);
    /* file already exists */
    if(hd != NULL){ perror("cp"); return -1; }

    int old_stdout = dup(STDIN_FILENO);
    dup2(pipe_fd, STDIN_FILENO);

    if(addTar(getenv("TARNAME"),tpcc) < 0){ perror("cp"); return -1; }

    dup2(old_stdout,STDIN_FILENO);
    close(pipe_fd);
  }else{
    /* normal reading in pipe and writing in file */
    int fd = open(file_name,O_WRONLY | O_CREAT,"0777");
    if(fd < 0) return -1;
    int size;
    while((size = read(pipe_fd,buf,BUF_SIZE)) > 0){
      if(write(fd,buf,size) < 0) return -1;
      memset(buf,'\0',BUF_SIZE);
    }
    close(fd); close(pipe_fd);
  }
  return 0;
}

int readFileWritePipe(char *file_name, int tar, int pipe_fd){
  char buf[BUF_SIZE];
  memset(buf,'\0',BUF_SIZE);

  if(tar){
    char *tpcc = tarpathconcat(file_name);
    struct posix_header* hd = getHeader(getenv("TARNAME"),tpcc);
    if(hd == NULL || hd->typeflag == '5'){ errno = EEXIST; perror("cp"); return -1; }

    int old_stdin = dup(STDIN_FILENO);
    dup2(pipe_fd, STDIN_FILENO);


    if(rdTar(getenv("TARNAME"),tpcc) < 0){ perror("cp"); return -1; }

    dup2(old_stdin, STDOUT_FILENO);
    close(pipe_fd);
  }else{
    /* normal reading in file and writing in pipe */
    int fd = open(file_name,O_RDONLY);
    if(fd < 0) return -1;
    int size;
    while((size = read(fd,buf,BUF_SIZE)) > 0){
      if(write(pipe_fd,buf,size) < 0) return -1;
      memset(buf,'\0',BUF_SIZE);
    }
    close(fd); close(pipe_fd);
  }
  return 0;
}

int cdTo(char *path, char* last_arg){
  char* s = pathminus(path,last_arg);
  if(cd(s) < 0){ return -1; }
  free(s);
  return 0;
}

char* pathminus(char *path, char *lastarg){
  char *s = malloc((strlen(path) - strlen(lastarg) + 1) * sizeof(char));
  memset(s,'\0',strlen(path) - strlen(lastarg) + 1);
  strncat(s,path,strlen(path) - strlen(lastarg));
  strcat(s,"\0");
  return s;
}

char *tarpathconcat(char *arg){
  int off = (arg[0] == '/')? 1 : 0;
  if(strlen(getenv("TARPATH")) <= 0) return arg + off;
  char *s = malloc(sizeof(char) * (strlen(getenv("TARPATH")) + strlen(arg) + ((off + 1)%2) + 1));
  memset(s,'\0',strlen(getenv("TARPATH")) + strlen(arg) - off + 1);
  strcat(s,getenv("TARPATH"));
  strcat(s,arg + off);
  strcat(s,"\0");
  return s;
}
