#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "cd.h"
#include "../useful.h"

#define BUF_SIZE 512

/* copies argv[0] to argv[1] */
int cp_2args(char**argv);
/* copies every file from {argv[0] ; ... ; argv[argc - 2]} to argv[argc - 1] */
int cp_nargs(char**argv, int argc);
/* if last arg of argv[1] is a dir, creates directory with same name in argv[2]
and then copies every files in argv[2] with cp_2args */
int cp_r(char** argv);
/* tries to cd to path minus lastarg */
int cdTo(char *path, char* last_arg);
/* reads data in pipe and writes it in file_name. tar specifies if its a tar or not */
int readPipeWriteFile(char *file_name, int tar, int pipe_fd);
/* same as above but reads in file and write in pipe */
int readFileWritePipe(char *file_name, int tar, int pipe_fd);
/* concatenate arg with env variable "TARPATH" */
char *tarpathconcat(char *arg);
int isDir(char *path);
void createDir(char **argv);

int main(int argc, char**argv){
  if(argc < 3) return -1;
  if(strcmp(argv[1],"-r") == 0){
    /* pas assez d'arguments */
    if(argc == 4){ return cp_r(argv + 2); }else{
      { perror("option non supportée.\n"); return -1; }
    }
  }
  if(argc == 3) return cp_2args(argv + 1);
  return cp_nargs(argv + 1, argc - 1);
}

int sameLevel(char *path){
  char *s = strchr(path,'/');
  if(s == NULL || s[1] == '\0') return 1;
  return 0;
}

int cp_r(char **argv){
  char *simplified_1 = getRealPath(argv[0]);
  char *simplified_2 = getRealPath(argv[1]);
  if(strstr(simplified_1,".tar") == NULL && strstr(simplified_2,".tar") == NULL)
    execlp("ls","ls","-r",simplified_1,simplified_2,NULL);
  /* argv[0] n'est pas un dossier (= on ne peut pas y acceder ) */
  /* alors on tente juste de copier argv[0] à argv[1] */

  if(isDir(argv[0]) != 1) return cp_2args(argv);
  /* sinon on créer le dossier à l'adresse p2 */
  // TODO : VERIFICATION NON EXISTENCE DU DOSSIER
  int wait, rpid;
  rpid = fork();
  switch(rpid){
    case -1 :
    { perror("cp_r\n"); return -1; }
    case 0 : createDir(argv); exit(0);
    default : waitpid(rpid, &wait,0); break;
  }
  // Pour le retour
  char *pwd = getcwd(NULL,0); char *tarname = getenv("TARNAME");
  char *tarpath = getenv("TARPATH");

  if(cd(argv[0]) < 0)
  { perror("fichier innaccessible.\n"); return -1; }

  // On parcourt maintenant tout les fichiers appartenant à ce dossier
  // Cas d'un tar
  // On ajoute les noms des fichiers a filenames
  char *filenames[512];
  int index = 0;
  if(strlen(getenv("TARNAME")) > 0){
    int fd = open(getenv("TARNAME"),O_RDONLY);
    char *path = getenv("TARPATH");
    if(fd < 0)
    { perror("erreur lors de l'ouverture du tar.\n"); return -1; }

    while(1){
      struct posix_header tampon;
      if(read(fd, &tampon, sizeof(struct posix_header)) < 0)
      { perror("erreur lors de  la lecture du tar.\n"); return -1; }

      /* if its empty, we stop */
      if(isEmpty(&tampon)) break;

      /* same prefix */
      if(!strncmp(tampon.name,path,strlen(path))){
        /* are on 'same level' (and not the same)*/
        if(strcmp(tampon.name,path) != 0 && sameLevel(tampon.name + strlen(path)))
        filenames[index++] = strdup(tampon.name + strlen(path));
      }

      unsigned int filesize;
      sscanf(tampon.size,"%o", &filesize);

      /* and size of its blocs */
      int s = (filesize + 512 - 1)/512;
      /* we read them if order to "ignore them" (we SHOULD use seek here) */
      char temp[s * BLOCKSIZE];
      read(fd, temp, s * BLOCKSIZE);
    }
  }else{
  // Cas normal
    DIR *dir = opendir(".");
    struct dirent *lecture;
    while ((lecture = readdir(dir)))
    {
      if(strcmp(lecture->d_name,".") && strcmp(lecture->d_name,".."))
        filenames[index++] = strdup(lecture->d_name);
    }
  }
  chdir(pwd); setenv("TARNAME",tarname,1); setenv("TARPATH",tarpath,1);
  for(int i = 0; i < index; i++){
    char argv0[strlen(argv[0]) + 1 + strlen(filenames[i]) + 1];
    memset(argv0,0,strlen(argv[0]) + 1 + strlen(filenames[i]) + 1);
    sprintf(argv0,"%s/%s",argv[0],filenames[i]);
    // On concatene tout
    char argv1[strlen(argv[1]) + 1 + strlen(filenames[i]) + 1];
    memset(argv1,0,strlen(argv[1]) + 1 + strlen(filenames[i]) + 1);
    sprintf(argv1,"%s/%s",argv[1],filenames[i]);

    char *newargv[2] = {argv0,argv1};
    int w;

    // et on rapelle recursivement sur chaque fichier
    int r = fork();
    switch(r){
      case -1 : perror("cp_r\n"); return -1;
      case 0 : exit(cp_r(newargv));
      default : waitpid(r,&w, 0);
      if(WEXITSTATUS(w) == 255) return -1;
      break;
    }
  }
  return 1;
}

void createDir(char **argv){
  /* on récupère le path ou sont stockés les fonctions sur les tars */
  char *pathname = getenv("TARCMDSPATH");
  /* on y concatene la commande voulue */
  char pathpluscmd[strlen(pathname) + 1 + strlen("mkdir")];
  memset(pathpluscmd,0,strlen(pathname) + 1 + strlen("mkdir"));
  sprintf(pathpluscmd,"%s/%s",pathname,"mkdir");

  /* et on exec */
  execl(pathpluscmd,argv[1]);
  exit(-1);
}

int isDir(char *path){
  int w;
  int r = fork();
  switch(r){
    case -1 : perror("isDir.\n"); return -1;
    case 0 : exit(cd(path));
    default : break;
  }
  waitpid(r,&w,0);
  return WEXITSTATUS(w);
}

int cp_2args(char **argv){
  // Pour cp dans le cas d'un cp normal (les chemins sont en fait des chemins sans tar)
  char *simplified_1 = getRealPath(argv[0]);
  char *simplified_2 = getRealPath(argv[1]);
  if(strstr(simplified_1,".tar") == NULL && strstr(simplified_2,".tar") == NULL)
    execlp("ls","ls",simplified_1,simplified_2,NULL);
  char *last_arg_1 = getLastArg(simplified_1);
  char *last_arg_2 = getLastArg(simplified_2);

  int pipe_fd[2];
  pipe(pipe_fd);

  switch(fork()){
    case - 1 : perror("cp\n"); return -1;
    case 0 :
    close(pipe_fd[0]);
    if(strcmp(argv[0],last_arg_1)){
      if(cdTo(argv[0],last_arg_1) < 0)
      { perror("chemin innaccessible.\n"); return -1; }
    }
    // SI C'EST UN DOSSIER ET QUE L'OPTION '-R' N'EST PAS PRECISE, PROPOSER
    readFileWritePipe(last_arg_1, strlen(getenv("TARNAME")) > 0, pipe_fd[1]);
    exit(0);
    default :
    close(pipe_fd[1]);
    if(strcmp(argv[1],last_arg_2)){
      if(cdTo(argv[1],last_arg_2) < 0)
      { perror("chemin innaccessible.\n"); return -1; }
    }
    // si on "est" dans un tar
    if(strlen(getenv("TARNAME")) > 0){
      struct posix_header* h = getHeader(getenv("TARNAME"),getLastArg(simplified_2));
      if(h != NULL && h->typeflag == '5'){
        cd(last_arg_2);
        last_arg_2 = last_arg_1;
      }
    }else{
    // sinon
      DIR *dir = opendir(getLastArg(simplified_2));
      if(dir != NULL){
        cd(last_arg_2);
        last_arg_2 = last_arg_1;
      }
    }
    // Si le fichier existe déjà -> erreur
    if(existsTP(last_arg_2)){ perror("le fichier existe déjà.\n"); return -1; }
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
      case - 1 : perror("cp\n"); return -1;
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

    if(addTar(getenv("TARNAME"),tpcc,'0') < 0){ perror("cp"); return -1; }

    dup2(old_stdout,STDIN_FILENO);
    close(pipe_fd);
  }else{
    /* normal reading in pipe and writing in file */
    int fd = open(file_name,O_WRONLY | O_CREAT, 0644);
    if(fd < 0)
    { perror("erreur lors de l'ouverture du tar.\n"); return -1; }
    int size;
    while((size = read(pipe_fd,buf,BUF_SIZE)) > 0){
      if(write(fd,buf,size) < 0)
      { perror("erreur lors de la lecture dans le tar.\n"); return -1; }
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
    if(fd < 0)
    { perror("erreur lors de l'ouverture du tar.\n"); return -1; }
    int size;
    while((size = read(fd,buf,BUF_SIZE)) > 0){
      if(write(pipe_fd,buf,size) < 0)
      { perror("erreur lors de l'ecriture dans le tar.\n"); return -1; }
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
