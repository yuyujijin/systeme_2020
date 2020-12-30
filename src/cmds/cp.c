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
int readPipeWriteFile(struct special_path sp, int tar, int pipe_fd);
/* same as above but reads in file and write in pipe */
int readFileWritePipe(struct special_path sp, int tar, int pipe_fd);
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
  struct special_path sp1 = special_path_maker(simplified_1);
  free(simplified_1);

  char *simplified_2 = getRealPath(argv[1]);
  struct special_path sp2 = special_path_maker(simplified_2);
  free(simplified_2);

  // Cas des arguments pas dans un tar
  if(strlen(sp1.tar_name) == 0 && strlen(sp2.tar_name) == 0){
    char p1[strlen(sp1.path) + 2];
    memset(p1,0,strlen(sp1.path) + 2);
    sprintf(p1,"/%s",sp1.path);
    if(p1[strlen(p1) - 1] == '/') p1[strlen(p1) - 1] = '\0';

    char p2[strlen(sp2.path) + 2];
    memset(p2,0,strlen(sp2.path) + 2);
    sprintf(p2,"/%s",sp2.path);
    if(p2[strlen(p2) - 1] == '/') p2[strlen(p2) - 1] = '\0';
    execlp("cp","cp","-r",p1,p2,NULL);
    exit(-1);
  }
  // Cas d'une copie fichier .tar vers .tar
  if(strlen(sp1.tar_name) != 0 && strlen(sp2.tar_name) != 0
    && strlen(sp1.tar_path) == 0 && strlen(sp2.tar_path) == 0){
      char p1[strlen(sp1.path) + strlen(sp1.tar_name) + 2];
      memset(p1,0,strlen(sp1.path) + strlen(sp1.tar_name) + 2);
      sprintf(p1,"/%s%s",sp1.path,sp1.tar_name);
      if(p1[strlen(p1) - 1] == '/') p1[strlen(p1) - 1] = '\0';

      char p2[strlen(sp2.path) + strlen(sp2.tar_name) + 2];
      memset(p2,0,strlen(sp2.path) + strlen(sp2.tar_name) + 2);
      sprintf(p2,"/%s%s",sp2.path,sp2.tar_name);
      if(p2[strlen(p2) - 1] == '/') p2[strlen(p2) - 1] = '\0';
      execlp("cp","cp","-r",p1,p2,NULL);
      exit(-1);
  }

  char p1[strlen(sp1.path) + strlen(sp1.tar_name) + 2];
  memset(p1,0,strlen(sp1.path) + strlen(sp1.tar_name) + 2);
  sprintf(p1,"/%s%s",sp1.path,sp1.tar_name);
  if(p1[strlen(p1) - 1] == '/') p1[strlen(p1) - 1] = '\0';

  if(strlen(sp1.tar_path) > 0)
    sp1.tar_path[strlen(sp1.tar_path) - 1] = '\0';
  if(strlen(sp2.tar_path) > 0)
    sp2.tar_path[strlen(sp2.tar_path) - 1] = '\0';

  // Est ce un dossier? (si ce n'est pas le cas cas on rappel juste cp_2args)
  if(strlen(sp1.tar_name) > 0 && strlen(sp1.tar_path) > 0){
    struct posix_header *ph = getHeader(p1,sp1.tar_path);

    if(ph == NULL){
      perror("fichier non existant.\n");
      return -1;
    }

    if(ph->typeflag != '5'){
      return cp_2args(argv);
    }
  }else if(strlen(sp1.tar_name) == 0){
    int fd = open(p1,O_RDONLY);
    if(fd < 0){
      perror("fichier non existant.\n");
      return -1;
    }
    // On récupère les stats
    struct stat statbuf;
    // Si il existe, et que c'est un dossier
    if(fstat(fd,&statbuf) != -1){
      if(!S_ISDIR(statbuf.st_mode)){
        close(fd);
        return cp_2args(argv);
      }
    }
    close(fd);
  }

  /* sinon on créer le dossier à l'adresse p2 */
  int wait, rpid;
  rpid = fork();
  switch(rpid){
    case -1 :
    { perror("cp_r\n"); return -1; }
    case 0 : createDir(argv); exit(-1);
    default : waitpid(rpid, &wait,0); break;
  }
  if(WEXITSTATUS(wait) != 1){ perror("erreur lors de la création du dossier."); return -1; }

  // On parcourt maintenant tout les fichiers appartenant à ce dossier
  // Cas d'un tar
  // On ajoute les noms des fichiers a filenames
  char *filenames[512];
  int index = 0;
  if(strlen(sp1.tar_name) > 0){
    int fd = open(p1,O_RDONLY);
    if(fd < 0)
      { perror("erreur lors de l'ouverture du tar.\n"); return -1; }

    char path[strlen(sp1.tar_path) + 2];
    memset(path,0,strlen(sp1.tar_path) + 2);
    if(strlen(sp1.tar_path) > 0) sprintf(path,"%s/",sp1.tar_path);

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
      lseek(fd,s * BLOCKSIZE,SEEK_CUR);
    }
  }else{
  // Cas normal
    DIR *dir = opendir(p1);
    struct dirent *lecture;
    while ((lecture = readdir(dir)))
    {
      if(strcmp(lecture->d_name,".") && strcmp(lecture->d_name,".."))
        filenames[index++] = strdup(lecture->d_name);
    }
  }

  for(int i = 0; i < index; i++){
    // On concatene tout
    char argv0[strlen(argv[0]) + 1 + strlen(filenames[i]) + 1];
    memset(argv0,0,strlen(argv[0]) + 1 + strlen(filenames[i]) + 1);
    strcat(argv0,argv[0]);
    if(argv[0][strlen(argv[0]) - 1] != '/') strcat(argv0,"/");
    strcat(argv0,filenames[i]);

    // On concatene tout
    char argv1[strlen(argv[1]) + 1 + strlen(filenames[i]) + 1];
    memset(argv1,0,strlen(argv[1]) + 1 + strlen(filenames[i]) + 1);
    strcat(argv1,argv[1]);
    if(argv[1][strlen(argv[1]) - 1] != '/') strcat(argv1,"/");
    strcat(argv1,filenames[i]);

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
  char *args[3] = {"mkdir",argv[1],NULL};
  execv(pathpluscmd,args);
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

char *getSpecialPathLastArg(struct special_path sp){
  if(strlen(sp.tar_name) > 0){
    if(strlen(sp.tar_path) > 0){
      return getLastArg(sp.tar_path);
    }
    return sp.tar_name;
  }
  sp.path[strlen(sp.path) - 1] = '\0';
  return getLastArg(sp.path);
}

int cp_2args(char **argv){
  char *simplified_1 = getRealPath(argv[0]);
  struct special_path sp1 = special_path_maker(simplified_1);
  free(simplified_1);

  char *simplified_2 = getRealPath(argv[1]);
  struct special_path sp2 = special_path_maker(simplified_2);
  free(simplified_2);

  // Cas des arguments pas dans un tar
  if(strlen(sp1.tar_name) == 0 && strlen(sp2.tar_name) == 0){
    char p1[strlen(sp1.path) + 2];
    memset(p1,0,strlen(sp1.path) + 2);
    sprintf(p1,"/%s",sp1.path);
    if(p1[strlen(p1) - 1] == '/') p1[strlen(p1) - 1] = '\0';

    char p2[strlen(sp2.path) + 2];
    memset(p2,0,strlen(sp2.path) + 2);
    sprintf(p2,"/%s",sp2.path);
    if(p2[strlen(p2) - 1] == '/') p2[strlen(p2) - 1] = '\0';
    execlp("cp","cp",p1,p2,NULL);
  }

  char p1[strlen(sp1.path) + strlen(sp1.tar_name) + 2];
  memset(p1,0,strlen(sp1.path) + strlen(sp1.tar_name) + 2);
  sprintf(p1,"/%s%s",sp1.path,sp1.tar_name);
  if(p1[strlen(p1) - 1] == '/') p1[strlen(p1) - 1] = '\0';

  if(strlen(sp1.tar_path) > 0)
    sp1.tar_path[strlen(sp1.tar_path) - 1] = '\0';
  if(strlen(sp2.tar_path) > 0)
    sp2.tar_path[strlen(sp2.tar_path) - 1] = '\0';

  // Le premier argument existe ?
  if(strlen(sp1.tar_name) > 0){
    struct posix_header *ph = getHeader(p1,sp1.tar_path);

    if(strlen(sp1.tar_path) == 0){
      perror("est un dossier. (vous avez peut être oublié l'option '-r'?)");
      return -1;
    }

    if(ph == NULL){
      perror("fichier non existant.\n");
      return -1;
    }

    if(ph->typeflag == '5'){
      perror("est un dossier. (vous avez peut être oublié l'option '-r'?)");
      return -1;
    }
  }else{
    int fd = open(p1,O_RDONLY);
    if(fd < 0){
      perror("fichier non existant.\n");
      return -1;
    }
    // On récupère les stats
    struct stat statbuf;
    // Si il existe, et que c'est un dossier -> erreur
    if(fstat(fd,&statbuf) != -1){
      if(S_ISDIR(statbuf.st_mode)){
        perror("est un dossier. (vous avez peut être oublié l'option '-r'?)");
        return -1;
      }
    }
    close(fd);
  }

  // Si le second argument est un dossier, alors le premier argument
  // est copié dans le dossier sous le même nom
  char p2[strlen(sp2.path) + strlen(sp2.tar_name) + 2];
  memset(p2,0,strlen(sp2.path) + strlen(sp2.tar_name) + 2);
  sprintf(p2,"/%s%s",sp2.path,sp2.tar_name);
  if(p2[strlen(p2) - 1] == '/') p2[strlen(p2) - 1] = '\0';

  if(strlen(sp2.tar_name) > 0){
    struct posix_header *ph = getHeader(p2,sp2.tar_path);

    if(ph != NULL && ph->typeflag == '5'){
      char *p1lastArg = getSpecialPathLastArg(sp1);
      char newarg[strlen(sp2.tar_path) + strlen(p1lastArg) + 2];
      memset(newarg,0,strlen(p2) + strlen(p1lastArg) + 2);
      strcat(newarg,sp2.tar_path);
      if(strlen(sp2.tar_path) > 0) strcat(newarg,"/");
      strcat(newarg,p1lastArg);

      sp2.tar_path = strdup(newarg);
    }
  }else{
    // On récupère les stats
    struct stat statbuf;
    // Si il existe, et que c'est un dossier
    if(stat(p2,&statbuf) != -1){
      if(S_ISDIR(statbuf.st_mode)){
        char *p1lastArg = getSpecialPathLastArg(sp1);
        char newarg[strlen(p2) + strlen(p1lastArg) + 2];
        memset(newarg,0,strlen(p2) + strlen(p1lastArg) + 2);
        sprintf(newarg,"%s/%s",p2,p1lastArg);

        sp2.path = strdup(newarg);
      }
    }
  }

  int pipe_fd[2];
  pipe(pipe_fd);

  switch(fork()){
    case - 1 : perror("cp\n"); return -1;
    case 0 :
    close(pipe_fd[0]);
    readFileWritePipe(sp1, strlen(sp1.tar_name) > 0, pipe_fd[1]);
    freeSpecialPath(sp1);
    exit(0);
    default :
    close(pipe_fd[1]);
    readPipeWriteFile(sp2, strlen(sp2.tar_name) > 0, pipe_fd[0]);
    freeSpecialPath(sp2);
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

int readPipeWriteFile(struct special_path sp, int tar, int pipe_fd){
  char buf[BUF_SIZE];
  memset(buf,'\0',BUF_SIZE);

  char p[strlen(sp.path) + strlen(sp.tar_name) + 2];
  memset(p,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
  sprintf(p,"/%s%s",sp.path,sp.tar_name);
  if(p[strlen(p) - 1] == '/') p[strlen(p) - 1] = '\0';

  if(tar){
    struct posix_header* hd = getHeader(p,sp.tar_path);
    /* file already exists */
    if(hd != NULL){ perror("fichier existe déjà."); close(pipe_fd); return -1; }

    int old_stdout = dup(STDIN_FILENO);
    dup2(pipe_fd, STDIN_FILENO);

    if(addTar(p,sp.tar_path,'0') < 0){ perror("cp"); close(pipe_fd); return -1; }

    dup2(old_stdout,STDIN_FILENO);
    close(pipe_fd);
  }else{
    /* normal reading in pipe and writing in file */
    int fd = open(p,O_WRONLY | O_CREAT, 0644);
    if(fd < 0)
    { perror("erreur lors de l'ouverture du fichier.\n"); close(pipe_fd); return -1; }

    int size;
    while((size = read(pipe_fd,buf,BUF_SIZE)) > 0){
      if(write(fd,buf,size) < 0)
      { perror("erreur lors de la lecture dans le tar.\n"); close(pipe_fd); return -1; }
      memset(buf,'\0',BUF_SIZE);
    }
    close(fd); close(pipe_fd);
  }
  return 0;
}

int readFileWritePipe(struct special_path sp, int tar, int pipe_fd){
  char buf[BUF_SIZE];
  memset(buf,'\0',BUF_SIZE);

  char p[strlen(sp.path) + strlen(sp.tar_name) + 2];
  memset(p,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
  sprintf(p,"/%s%s",sp.path,sp.tar_name);
  if(p[strlen(p) - 1] == '/') p[strlen(p) - 1] = '\0';

  if(tar){
    struct posix_header* hd = getHeader(p,sp.tar_path);

    if(hd == NULL || hd->typeflag == '5'){ errno = EEXIST; perror("cp"); close(pipe_fd); return -1; }

    int old_stdin = dup(STDOUT_FILENO);
    dup2(pipe_fd, STDOUT_FILENO);

    if(rdTar(p,sp.tar_path) < 0){ perror("cp"); close(pipe_fd); return -1; }

    dup2(old_stdin, STDOUT_FILENO);
    close(pipe_fd);
  }else{
    /* normal reading in file and writing in pipe */
    int fd = open(p,O_RDONLY);
    if(fd < 0)
    { perror("erreur lors de l'ouverture du tar.\n"); close(pipe_fd); return -1; }
    int size;
    while((size = read(fd,buf,BUF_SIZE)) > 0){
      if(write(pipe_fd,buf,size) < 0)
      { perror("erreur lors de l'ecriture dans le tar.\n"); close(pipe_fd); return -1; }
      memset(buf,'\0',BUF_SIZE);
    }
    close(fd); close(pipe_fd);
  }
  return 0;
}
