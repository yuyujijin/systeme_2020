#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "cd.h"

#define BUF_SIZE 512

/* copies argv[0] to argv[1] */
int cp_2args(char**argv);
/* copies every file from {argv[0] ; ... ; argv[argc - 2]} to argv[argc - 1] */
int cp_nargs(char**argv, int argc);
/* if last arg of argv[1] is a dir, creates directory with same name in argv[2]
and then copies every files in argv[2] with cp_2args */
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
int isDir(char *path);
int cp_dir(char *dirname, char *path);

int main(int argc, char**argv){
  if(argc < 3) return -1;
  setenv("TARNAME","",1);
  setenv("TARPATH","",1);
  if(strcmp(argv[1],"-r") == 0){
    /* pas assez d'arguments */
    if(argc == 4){ return cp_r(argv + 2); }else{ return -1; }
  }
  if(argc == 3) return cp_2args(argv + 1);
  return cp_nargs(argv + 1, argc - 1);
}

int cp_r(char **argv){
  printf("%s : %s\n",argv[0],argv[1]);
  char *last_arg_2 = (strrchr(argv[1],'/') != NULL)? strrchr(argv[1],'/') : argv[1];
  /* argv[0] n'est pas un dossier (= on ne peut pas y acceder ) */
  /* alors on tente juste de copier argv[0] à argv[1] */
  if(isDir(argv[0]) != 1) return cp_2args(argv);
  /* sinon on créer le dossier à l'adresse p2 */
  /* UTILISER MKDIR QUAND ELLE SERA PRETE */
  int r = fork(), w;
  switch(r){
    case -1 : exit(-1);
    case 0 : exit(cp_dir(last_arg_2, pathminus(argv[1],last_arg_2)));
    default : waitpid(r,&w,0); break;
  }
  if(WEXITSTATUS(w) != 1) return -1;
  /* puis on parcours le dossier argv[1] pour tout x appartenant a argv[1],
  on rapelle cp_r (argv[0]/x, argv[1]/x) */
  /* on va pour ça fork, envoyer le fils en cd argv[1], et pour tout les fichiers
  /dossiers de argv[1], écrire leur nom dans un pipe précédé de la longueur du nom */
  int pipe_fd[2];
  pipe(pipe_fd);

  switch(fork()){
    case -1 : return -1;
    case 0 :
    close(pipe_fd[0]);
    if(cd(argv[0]) < 0) exit(-1);
    char *tarname = getenv("TARNAME");
    /* cas d'un tar */
    if(tarname != NULL && strlen(tarname) > 0){
      //TODO...
      // parcourir les fichiers du tar de la forme "tarpath_actuel/fichier"

    /* cas d'un dossier 'normal' */
    }else{
      // parcour normal à l'aide d'une struct DIR
      DIR *dir = opendir(".");
      if(dir == NULL){ return -1; }
      struct dirent *f;
      /* on parcours tout les fichiers du dossier et on écrit leur nom
      precedé de leur taille */
      while ((f = readdir(dir)) != NULL){
        if(strcmp(f->d_name,".") != 0 && strcmp(f->d_name,"..") != 0){
          char size[4];
          memset(size,0,4);
          sprintf(size,"%ld",strlen(f->d_name));
          write(pipe_fd[1],size,4);
          write(pipe_fd[1],f->d_name,strlen(f->d_name));
        }
      }
    }
    close(pipe_fd[1]);
    break;
    default :
    close(pipe_fd[1]);
    char buf[4];
    memset(buf,0,4);
    /* on lit une taille dans le pipe, puis un mot de la taille lu et on rappel recursivement dessus */
    while(read(pipe_fd[0],buf,4) > 0){
      int size = atoi(buf);
      char name[size + 1];
      memset(name,0,size + 1);
      read(pipe_fd[0],name,size);
      /* on créer un nouveau argv[0] et argv[1] de la forme argv[0]/nom et argv[1]/nom */
      char s_1[strlen(argv[0]) + strlen(name) + 2]; char s_2[strlen(argv[1]) + strlen(name) + 2];
      memset(s_1,0,strlen(argv[0]) + strlen(name) + 2);
      memset(s_2,0,strlen(argv[1]) + strlen(name) + 2);
      strcat(s_1,argv[0]); strcat(s_2,argv[1]);
      if(argv[0][strlen(argv[0]) - 1] != '/') strcat(s_1,"/");
      if(argv[1][strlen(argv[1]) - 1] != '/') strcat(s_2,"/");
      strcat(s_1,name); strcat(s_2,name);
      char *nargv[2] = {s_1,s_2};
      if(cp_r(nargv) < 0) return -1;
    }
    break;
  }
  return 1;
}

int isDir(char *path){
  int w;
  int r = fork();
  switch(r){
    case -1 : return -1;
    case 0 : exit(cd(path));
    default : break;
  }
  waitpid(r,&w,0);
  return WEXITSTATUS(w);
}

int cp_dir(char *dirname, char *path){
  if(cd(path) < 0) return -1;
  char *s = getenv("TARNAME");
  /* dans un tar */
  if(s != NULL && strlen(s) > 0){
    char *tpcc = tarpathconcat(dirname);
    int size = strlen(tpcc);
    /* dans le cas ou la dernière lettre n'est pas "/" */
    if(tpcc[strlen(tpcc) - 1] != '/') size += 2;
    char p[size];
    memset(p,0,size);
    strcat(p,tpcc);
    if(tpcc[strlen(tpcc) - 1] != '/') strcat(p,"/");
    strcat(p,"\0");
    if(exists(getenv("TARNAME"),p)) return -1;
    return addTar(getenv("TARNAME"),p,'5',1);
  }
  /* pas dans un tar */
  struct stat st = {0};
  /* si le dossier existe déjà */
  if(stat(dirname,&st) != -1) return -1;
  return mkdir(dirname,0700);
}

int cp_2args(char **argv){
  printf("arg1 : _%s_ arg2 : _%s_\n",argv[0],argv[1]);
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

    if(addTar(getenv("TARNAME"),tpcc,'0',0) < 0){ perror("cp"); return -1; }

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
