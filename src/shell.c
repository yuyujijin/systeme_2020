#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include "cmds/cd.h"
#include <ctype.h>
#include "tar_manipulation.h"
#include "useful.h"

#define MAX_SIZE 256
#define BOLDGREEN "\x1B[1;32m"
#define BOLDBLUE "\x1B[1;34m"
#define RESET "\x1B[0m"

char *required[] = {"pwd","mkdir","rmdir","mv","cp","rm","ls","cat",NULL};

/* begintrim trims the beginning, endtrim the end, trim both */
char *begintrim(char * s);
char *endtrim(char *s);
char *trim(char *s);


/* str_cut takes a input string of size length, and returns
a array of string containingthe sub-string of input_str delimited
by tokens (number of sub-string is given by arc) */
char** str_cut(char *input_str, char *tokens, int* argc);

/* execute cmd argv[0] with its args */
int execute_cmd(char**argv);

/* same w/ tar */
int execute_tar_cmd(char **argv);

void printcwd();

/*
  modify argv[i] if we are inside a tar. if we did cd a.tar/b and then ls c
  --> tar_path=a.tar/b && the commands that will be executed will be ls a.tar/b/c
  since we added the TARPATH in the arguments
*/
int add_tar_path_to_args(char **argv,int argc);

/* read_line read the next line from stdin */
char* read_line();

/* this function check if one of the args is looking INSIDE a tar */
int one_of_args_is_tar(char **argv);

int execute_redirection(int argc, char **argv);

int main(){
  /* environnement variable that store the additional path */
  setenv("TARPATH","",1);
  setenv("TARNAME","",1);

  char cwd[MAX_SIZE];
  memset(cwd,0,MAX_SIZE);
  getcwd(cwd,MAX_SIZE);
  char tarcmdspath[strlen(cwd) + strlen("/cmds")];
  memset(tarcmdspath,0,strlen(cwd) + strlen("/cmds"));
  sprintf(tarcmdspath,"%s/cmds",cwd);

  setenv("TARCMDSPATH",tarcmdspath,1);

  int backslashn = 1;

  while(1){
    printcwd();

    /* read next line */
    char *line = read_line();
    if(line == NULL) break;
    if(strlen(line) <= 1) continue;

    /* cut it in words array with space char delimiter */
    int argc;
    char **args = str_cut(line,"  ", &argc);
    if(args == NULL) return -1;

    free(line);

    if(strcmp(args[0],"exit") == 0){ free(args[0]); free(args); backslashn = 0; break; }

    /* specific case for cd, because we do not execute it */
    if(strcmp(args[0],"cd") == 0){
      if(args[1] == NULL) continue;
      if(cd(args[1]) < 0) perror("cd");
      continue;
    }

    /* in case we're in a tarball */
    int w;
    switch(fork()){
      case -1: return -1;
      case 0 : execute_redirection(argc,args); exit(-1);
      default : wait(&w); break;
    }

    // args[0] est le pointeur pour tout les args[i] (issues d'un strtok)
    // Tout les mots de args sont issues d'un strtok, pas besoin de free
    free(args[0]);
    free(args);
  }

  if(backslashn) write(STDOUT_FILENO,"\n",1);
  char *goodbye = "Merci d'avoir utilisé nos services !\n";
  write(STDOUT_FILENO,goodbye,strlen(goodbye));
  return 0;
}

void printcwd(){
  char bgnline[MAX_SIZE];
  memset(bgnline,0,MAX_SIZE);
  // On recupère le cwd (taille / 2 car plus petit)
  char cwd[MAX_SIZE / 2];
  memset(cwd,0,MAX_SIZE / 2);
  getcwd(cwd,MAX_SIZE / 2);
  char *separator = (strlen(getenv("TARNAME")) > 0)? "/" : "";
  sprintf(bgnline,"%s%s%s:%s%s%s%s%s%s%s$ ",BOLDGREEN,getlogin(),RESET,BOLDBLUE,cwd,separator,getenv("TARNAME"),separator,getenv("TARPATH"),RESET);

  write(STDIN_FILENO, bgnline, strlen(bgnline));
}

char *read_line(){
  char buf[MAX_SIZE];
  memset(buf,'\0',MAX_SIZE);
  if(read(STDIN_FILENO, buf, MAX_SIZE) <= 0) return NULL;

  char *s = malloc(sizeof(char) * (strlen(buf) + 1));
  memset(s,'\0',strlen(buf));
  strcpy(s,buf);

  return s;
}

char** str_cut(char *input_str, char *tokens, int* argc){
  *argc = 0;

  int i = 2;
  
  for(unsigned int j = 0; j < strlen(input_str); j++){
    for(unsigned int k = 0; k < strlen(tokens); k++){
      if(input_str[j] == tokens[k]){ i++; break; }
    }
  }

  char **words = (char **) malloc(sizeof(char*) * i);
  if(words == NULL) return NULL;
  words[0] = NULL;

  /* we go through the whole sentence */
  char *word = strtok(strdup(input_str),tokens);
  while(word != NULL){
    words[(*argc)++] = trim(word);
    words[(*argc)] = NULL;
    word = strtok(NULL,tokens);
  }
  free(word);
  (*argc)++;
  return words;
}

int execute_pipe_cmd(int argc, char **argv){
  // On compte le nombre de symbole pipe
  int pipelines = 1;
  for(int i = 0; argv[i] != NULL; i++) if(!strcmp(argv[i],"|")) pipelines++;

  // Puis on créer un tableau de tableau de string
  // dans lequel on insert les 'lignes' (sans les pipes)
  char *pipelines_args[pipelines][argc + 1];
  int c = 0, l = 0;
  for(int j = 0; argv[j] != NULL; j++){
    if(!strcmp(argv[j],"|")){
      pipelines_args[l][c] = NULL;
      c = 0; l++; continue;
    }
    pipelines_args[l][c++] = argv[j];
  }
  pipelines_args[l][c] = NULL;

  int nbr = pipelines;

  int pipefds[nbr][2];

  for(int i = 0; i < nbr; i++) pipe(pipefds[i]);

  int w;
  for(int i = 0; i < pipelines; i++){
    int r = fork();
    switch(r){
      case -1 : return -1;
      case 0 :
      if(i == 0) close(pipefds[0][0]);
      if(i == nbr - 1) close(pipefds[nbr - 1][1]);

      for(int j = 0; j < nbr; j++){
        if(j != i){
          close(pipefds[j][1]);
        }
        if(j != i - 1){
          close(pipefds[j][0]);
        }
      }

      int oldstdin = dup(STDIN_FILENO);

      if(i < nbr - 1) dup2(pipefds[i][1],STDOUT_FILENO);
      if(i > 0) dup2(pipefds[i-1][0],STDIN_FILENO);

      execute_cmd(pipelines_args[i]);

      char s[MAX_SIZE];
      memset(s,0,MAX_SIZE);
      sprintf(s,"%s : commande introuvable\n",pipelines_args[i][0]);

      dup2(oldstdin,STDIN_FILENO);
      write(STDIN_FILENO,s,strlen(s));
      exit(-1);
    default : close(pipefds[i][1]); if(i > 0) close(pipefds[i-1][0]); waitpid(r,&w,0); break;
    }
  }

  for(int i = 0; i < nbr; i++){ close(pipefds[i][0]); close(pipefds[i][1]); }
  for(int i = 0; i < argc; i++) free(argv[i]);
  return 1;
}

int execute_redirection(int argc, char **argv){
  /* on créer un tableau pour contenir la seule commande a exec */
  // On remarquera qu'on utilise les 'chemins spéciaux' (utilisé dans cd)
  // ici, plutôt que cd directement, car la situation ne nous permet pas de
  // changer de répertoire.
  char *argv_no_redirection[argc + 1];
  int j = 0;

  /* puis on parcourt tout les mots */
  for (int i = 0; i < argc - 1; i++) {
    /* si c'est '<', on essaie de rediriger stdin dans argv[i+1] */
    if (!strcmp(argv[i], "<")) {
      /* on récupère le 'vrai chemin', en créer un 'special path'
      (chemin, nom du tar et path dans le tar séparé) */
      char *p = getRealPath(argv[i+1]);
      special_path sp = special_path_maker(p);
      free(p);

      if(strlen(sp.tar_path) > 0) sp.tar_path[strlen(sp.tar_path) - 1] = '\0';

      /* le tar a ouvrir est a l'adresse "/" + pwd + nom du tar */
      char tarlocation[strlen(sp.path) + strlen(sp.tar_name) + 2];
      memset(tarlocation,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
      sprintf(tarlocation,"/%s%s",sp.path,sp.tar_name);

      /* si on est dans un tar */
      if(strlen(sp.tar_name) > 0){
        struct posix_header *ph = getHeader(tarlocation,sp.tar_path);

        if(ph == NULL){
	  free(ph);
	  freeSpecialPath(sp);
          perror("impossible d'ouvrir le fichier.\n");
          return -1;
        }

        if(ph->typeflag == '5'){
	  free(ph);
	  freeSpecialPath(sp);
          perror("est un dossier.");
          return -1;
        }

        /* puis on pipe rdTar dans l'entrée du pipe, et la sortie sur STDIN */
          int pipefd[2];
          pipe(pipefd);
          switch(fork()){
            case -1 : return -1;
            case 0 :
            close(pipefd[0]);
            dup2(pipefd[1],STDOUT_FILENO);
            rdTar(tarlocation,sp.tar_path);
            exit(0);
            default :
            close(pipefd[1]);
            dup2(pipefd[0],STDIN_FILENO);
            break;
          }
        }else{
          char chemin[strlen(sp.path) + 2];
          memset(chemin,0,strlen(sp.path) + 2);
          sprintf(chemin,"/%s",sp.path);
          chemin[strlen(chemin) - 1] = '\0';

          argv[i + 1] = chemin;

          // On récupère les stats
          struct stat statbuf;
          // Si il existe, et que c'est un dossier -> erreur
          if(stat(argv[i+1],&statbuf) != -1){
            if(S_ISDIR(statbuf.st_mode)){
	      freeSpecialPath(sp);
              perror("est un dossier.");
              return -1;
            }
          }
          int stdin = open(argv[i+1], O_RDONLY, 0644);
          if(stdin < 0) {
             perror("impossible d'ouvrir le fichier.\n");
	     freeSpecialPath(sp);
             return -1;
          }
          dup2(stdin, STDIN_FILENO);
          close(stdin);
      }
      freeSpecialPath(sp);
      i++;
      continue;
    }

    /*
    si c'est '>', on essaie de rediriger stdout dans argv[i+1]
    si c'est '2>', on essaie de rediriger stderr dans argv[i+1]
    */
    if (!strcmp(argv[i],"2>") || !strcmp(argv[i], ">")) {
      /* on récupère le 'vrai chemin', en créer un 'special path'
      (chemin, nom du tar et path dans le tar séparé) */
      char *p = getRealPath(argv[i+1]);
      special_path sp = special_path_maker(p);
      free(p);

      if(strlen(sp.tar_path) > 0) sp.tar_path[strlen(sp.tar_path) - 1] = '\0';

      /* le tar a ouvrir est a l'adresse "/" + pwd + nom du tar */
      char tarlocation[strlen(sp.path) + strlen(sp.tar_name) + 2];
      memset(tarlocation,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
      sprintf(tarlocation,"/%s%s",sp.path,sp.tar_name);

      /* si on est dans un tar */
      if(strlen(sp.tar_name) > 0){
        struct posix_header *ph = getHeader(tarlocation,sp.tar_path);

        if(ph != NULL && ph->typeflag == '5'){
	  free(ph);
	  freeSpecialPath(sp);
          perror("est un dossier.");
          return -1;
        }

        /* puis on pipe la sortie du pipe sur STDIN, et on pipe l'entrée sur stdout */
        int pipefd[2];
        pipe(pipefd);
        switch(fork()){
          case -1 : return -1;
          case 0 :
          close(pipefd[1]);
          // On supprime le fichier pour simuler l'effet du O_TRUNCAT
          rmTar(tarlocation,sp.tar_path);
          dup2(pipefd[0],STDIN_FILENO);
          addTar(tarlocation,sp.tar_path,'0');
          exit(0);
          default :
          close(pipefd[0]);
          if(!strcmp(argv[i], ">")) dup2(pipefd[1],STDOUT_FILENO);
          if(!strcmp(argv[i], "2>")) dup2(pipefd[1],STDERR_FILENO);
          break;
        }
      }else{
        char chemin[strlen(sp.path) + 2];
        memset(chemin,0,strlen(sp.path) + 2);
        sprintf(chemin,"/%s",sp.path);
        chemin[strlen(chemin) - 1] = '\0';

        argv[i + 1] = chemin;

        // On récupère les stats
        struct stat statbuf;
        // Si il existe, et que c'est un dossier -> erreur
        if(stat(argv[i+1],&statbuf) != -1){
          if(S_ISDIR(statbuf.st_mode)){
	    freeSpecialPath(sp);
            perror("est un dossier.");
            return -1;
          }
        }
        int stdout = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (stdout < 0) {
	  freeSpecialPath(sp);
            perror("impossible de créer le fichier.\n");
            return -1;
        }
        if(!strcmp(argv[i], ">")) dup2(stdout, STDOUT_FILENO);
        if(!strcmp(argv[i], "2>")) dup2(stdout, STDERR_FILENO);
        close(stdout);
      }
      freeSpecialPath(sp);
      i++;
      continue;
    }

    /* comme au dessus, mais en mode APPEND */
    if (!strcmp(argv[i],"2>>") || !strcmp(argv[i], ">>")) {
      /* on récupère le 'vrai chemin', en créer un 'special path'
      (chemin, nom du tar et path dans le tar séparé) */
      char *p = getRealPath(argv[i+1]);
      special_path sp = special_path_maker(p);
      free(p);

      if(strlen(sp.tar_path) > 0) sp.tar_path[strlen(sp.tar_path) - 1] = '\0';

      /* le tar a ouvrir est a l'adresse "/" + pwd + nom du tar */
      char tarlocation[strlen(sp.path) + strlen(sp.tar_name) + 2];
      memset(tarlocation,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
      sprintf(tarlocation,"/%s%s",sp.path,sp.tar_name);

      /* si on est dans un tar */
      if(strlen(sp.tar_name) > 0){
        struct posix_header *ph = getHeader(tarlocation,sp.tar_path);

        if(ph == NULL){
	  free(ph);
	  freeSpecialPath(sp);
          perror("impossible d'ouvrir le fichier.\n");
          return -1;
        }

        if(ph->typeflag == '5'){
	  free(ph);
	  freeSpecialPath(sp);
          perror("est un dossier.");
          return -1;
        }

        int pipefd[2];
        pipe(pipefd);
        switch(fork()){
          case -1 : return -1;
          case 0 :
          close(pipefd[1]);
          dup2(pipefd[0],STDIN_FILENO);
          appendTar(tarlocation,sp.tar_path);
          exit(0);
          default :
          close(pipefd[0]);
          if(!strcmp(argv[i], ">>")) dup2(pipefd[1],STDOUT_FILENO);
          if(!strcmp(argv[i], "2>>")) dup2(pipefd[1],STDERR_FILENO);
          break;
        }
      }else{
        char chemin[strlen(sp.path) + 2];
        memset(chemin,0,strlen(sp.path) + 2);
        sprintf(chemin,"/%s",sp.path);
        chemin[strlen(chemin) - 1] = '\0';

        argv[i + 1] = chemin;

        // On récupère les stats
        struct stat statbuf;
        // Si il existe, et que c'est un dossier -> erreur
        if(stat(argv[i+1],&statbuf) != -1){
          if(S_ISDIR(statbuf.st_mode)){
	    freeSpecialPath(sp);
            perror("est un dossier.");
            return -1;
          }
        }
        int concat = open(argv[i + 1], O_CREAT | O_RDWR | O_APPEND, 0644);
        if (concat < 0) {
	  freeSpecialPath(sp);
            perror("impossible de concatener au fichier.\n");
            return -1;
        }
        if(!strcmp(argv[i],">>")) dup2(concat, STDOUT_FILENO);
        if(!strcmp(argv[i],"2>>")) dup2(concat, STDERR_FILENO);

        close(concat);
      }
      freeSpecialPath(sp);
      i++;
      continue;
    }

    /* sortie erreur dans la sortie standard */
    if (!strcmp(argv[i],"2>&1")) {
      dup2(STDIN_FILENO,STDERR_FILENO);
      i++; continue;
    }

    /* sinon, on ajoute l'argument au tableau des executions */
    if(j > 0){
       argv_no_redirection[j++] = path_simplifier(argv[i]);
     }else{
       argv_no_redirection[j++] = argv[i];
     }
  }

  /* dernier argument a null (pour exec) */
  argv_no_redirection[j] = NULL;
  return execute_pipe_cmd(j, argv_no_redirection);
}

int requiredCmds(char *s){
  for(int i = 0; required[i] != NULL; i++){
    if(!strcmp(trim(s),required[i])) return 1;
  }
  return 0;
}

int execute_cmd(char**argv){
  if((strlen(getenv("TARNAME")) > 0 || one_of_args_is_tar(argv))
  && requiredCmds(argv[0])) return execute_tar_cmd(argv);
  execvp(argv[0], argv);

  char err[MAX_SIZE];
  memset(err,0,MAX_SIZE);
  sprintf(err,"Commande %s inconnue\n",argv[0]);
  write(STDERR_FILENO,err,strlen(err));

  exit(-1);
}

int execute_tar_cmd(char**argv){
  /* on récupère le path ou sont stockés les fonctions sur les tars */
  char *pathname = getenv("TARCMDSPATH");
  /* on y concatene la commande voulue */
  char pathpluscmd[strlen(pathname) + 1 + strlen(argv[0])];
  memset(pathpluscmd,0,strlen(pathname) + 1 + strlen(argv[0]));
  sprintf(pathpluscmd,"%s/%s",pathname,argv[0]);

  /* et on exec */
  execv(pathpluscmd,argv);

  /* cas d'une erreur */
  char err[MAX_SIZE];
  memset(err,0,MAX_SIZE);
  sprintf(err,"Commande %s inconnue\n",argv[0]);
  write(STDERR_FILENO,err,strlen(err));

  exit(-1);
}

int one_of_args_is_tar(char**argv){
  for(int i = 1; argv[i] != NULL; i++){
    if(strstr(argv[i],".tar") != NULL) return 1;
  }
  return 0;
}

char *begintrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *endtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return endtrim(begintrim(s));
}
