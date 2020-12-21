#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "cmds/cd.h"
#include <ctype.h>
#include "tar_manipulation.h"
#include "useful.h"

#define MAX_SIZE 256
#define BOLDGREEN "\x1B[1;32m"
#define BOLDBLUE "\x1B[1;34m"
#define RESET "\x1B[0m"

/* begintrim trims the beginning, endtrim the end, trim both */
char *begintrim(char * s);
char *endtrim(char *s);
char *trim(char *s);


/* str_cut takes a input string of size length, and returns
a array of string containingthe sub-string of input_str delimited
by tokens (number of sub-string is given by arc) */
char** str_cut(char *input_str, char *tokens, int* argc);

/* execute cmd argv[0] with its args */
int execute_cmd(int argc, char**argv);

/* same w/ tar */
int execute_tar_cmd(char **argv);

void printcwd();

/* read_line read the next line from stdin */
char* read_line();

/* this function check if one of the args is looking INSIDE a tar */
int one_of_args_is_tar(int argc,char **argv);

int execute_redirection(int argc, char **argv);

int main(){
  char* line;

  /* environnement variable that store the additional path */
  setenv("TARPATH","",1);
  setenv("TARNAME","",1);

  char tarcmdspath[strlen(getcwd(NULL,0)) + strlen("/cmds")];
  memset(tarcmdspath,0,strlen(getcwd(NULL,0)) + strlen("/cmds"));
  sprintf(tarcmdspath,"%s/cmds",getcwd(NULL,0));

  setenv("TARCMDSPATH",tarcmdspath,1);

  while(1){
    printcwd();

    /* read next line */
    line = read_line();
    if(line == NULL) break;

    /* cut it in words array with space char delimiter */
    int argc;
    char **args = str_cut(line," ", &argc);
    if(args == NULL) return -1;

    free(line);

    if(strcmp(args[0],"exit") == 0) exit(0);

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

    free(args);
  }

  write(STDOUT_FILENO,"\n",2);
  return 0;
}

void printcwd(){
  char bgnline[MAX_SIZE];
  char *cwd = getcwd(NULL, 0);
  char *separator = (strlen(getenv("TARNAME")) > 0)? "/" : "";
  sprintf(bgnline,"%s%s%s:%s%s%s%s%s%s%s$ ",BOLDGREEN,getlogin(),RESET,BOLDBLUE,cwd,separator,getenv("TARNAME"),separator,getenv("TARPATH"),RESET);

  write(STDIN_FILENO, bgnline, strlen(bgnline));

  free(cwd);
}

char *read_line(){
  char buf[MAX_SIZE];
  memset(buf,'\0',MAX_SIZE);
  read(STDIN_FILENO, buf, MAX_SIZE);

  char *s = malloc(sizeof(char) * (strlen(buf)));
  memset(s,'\0',strlen(buf));
  strcpy(s,buf);

  return s;
}

char** str_cut(char *input_str, char *tokens, int* argc){
  //printf("je coupe : _%s_ avec le token : _%s_\n",input_str,tokens);
  *argc = 0;
  char **words = (char **) malloc(sizeof(char*));
  if(words == NULL) return NULL;

  /* we go through the whole sentence */
  char *word = strtok(strdup(input_str),tokens);
  while(word != NULL){
    words[(*argc)++] = trim(word);
    words = realloc(words,((*argc) + 1) * sizeof(char*));
    words[(*argc)] = NULL;
    word = strtok(NULL,tokens);
  }
  (*argc)++;
  return words;
}


int execute_redirection(int argc, char **argv){
  /* on créer un tableau pour contenir la seule commande a exec */
  char *argv_no_redirection[argc + 1];
  int j = 0;

  /* puis on parcourt tout les mots */
  for (int i = 0; i < argc - 1; i++) {
    /* si c'est '<', on essaie de rediriger stdin dans argv[i+1] */
    if (!strcmp(argv[i], "<")) {
        int stdin = open(argv[++i], O_RDONLY, 0644);
        if(stdin < 0) {
            perror("impossible d'ouvrir le fichier.\n");
            return -1;
        }
        dup2(stdin, STDIN_FILENO);
        close(stdin);
        continue;
    }

    /*
    si c'est '>', on essaie de rediriger stdout dans argv[i+1]
    si c'est '2>', on essaie de rediriger stderr dans argv[i+1]
    */
    if (!strcmp(argv[i],"2>") || !strcmp(argv[i], ">")) {
        int stdout = open(argv[++i], O_WRONLY | O_CREAT, 0644);
        if (stdout < 0) {
            perror("impossible de créer le fichier.\n");
            return -1;
        }
        if(!strcmp(argv[i - 1], ">")) dup2(stdout, STDOUT_FILENO);
        if(!strcmp(argv[i - 1], "2>")) dup2(stdout, STDERR_FILENO);
        close(stdout);
        continue;
    }

    /* comme au dessus, mais en mode APPEND */
    if (!strcmp(argv[i],"2>>") || !strcmp(argv[i], ">>")) {
        int concat = open(argv[++i], O_CREAT | O_RDWR | O_APPEND, 0644);
        if (concat < 0) {
            perror("impossible de concatener au fichier.\n");
            return -1;
        }
        if(!strcmp(argv[i - 1],">>")) dup2(concat, STDOUT_FILENO);
        if(!strcmp(argv[i - 1],"2>>")) dup2(concat, STDERR_FILENO);

        close(concat);
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
  return execute_cmd(j, argv_no_redirection);
}

int execute_cmd(int argc, char**argv){
  if(strlen(getenv("TARNAME")) > 0 || one_of_args_is_tar(argc, argv)) return execute_tar_cmd(argv);
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

int one_of_args_is_tar(int argc, char**argv){
  for(int i = 1; i < argc; i++){
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
