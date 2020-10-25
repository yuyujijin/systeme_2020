#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "cmds/cd.h"

#define MAX_SIZE 256
#define BOLDGREEN "\x1B[1;32m"
#define BOLDBLUE "\x1B[1;34m"
#define RESET "\x1B[0m"

/* str_cut takes a input string of size length, and returns
a array of string containingthe sub-string of input_str delimited
by tokens (number of sub-string is given by arc) */
char** str_cut(char *input_str, char token,size_t length, int* argc);

/* execute cmd argv[0] with its args */
int execute_cmd(char **argv);

/* same w/ tar */
int execute_tar_cmd(char **argv);

/* read_line read the next line from stdin */
char* read_line();

/* this function check if one of the args is looking INSIDE a tar */
int one_of_args_is_tar(char **argv, int argc);

int main(){
  char* line;

  /* environnement variable that store the additional path */
  setenv("TARPATH","",1);

  while(1){
    char bgnline[256];
    char *cwd = getcwd(NULL, 0);
    cwd = realloc(cwd,strlen(cwd) + strlen(getenv("TARPATH")) + 2);
    strcat(cwd,"/");
    strcat(cwd,getenv("TARPATH"));
    sprintf(bgnline,"%s%s%s:%s%s%s$ ",BOLDGREEN,getlogin(),RESET,BOLDBLUE,cwd,RESET);

    write(STDIN_FILENO, bgnline, strlen(bgnline));

    /* read next line */
    line = read_line();
    if(line == NULL) break;

    /* cut it in words array with space char delimiter */
    int argc;
    char **args = str_cut(line,' ', strlen(line), &argc);
    if(args == NULL) return -1;

    if(strcmp(args[0],"exit") == 0) exit(0);

    /* specific case for cd, because we do not execute it */
    if(strcmp(args[0],"cd") == 0){
      if(args[1] == NULL) continue;
      if(cd(args[1]) < 0) perror("cd");
      continue;
    }

    /* in case we're in a tarball */
    if(strstr(getenv("TARPATH"),".tar") != NULL
       || one_of_args_is_tar(args + 1, argc -1)){
      if(execute_tar_cmd(args) < 0)
	printf("Commande %s non reconnue\n",args[0]);
    }else{
      if(execute_cmd(args) < 0)
	printf("Commande %s non reconnue\n",args[0]);
    }
  }

  write(STDIN_FILENO,"\n",2);
  return 0;
}

char *read_line(){
  char buf[MAX_SIZE];
  size_t size;

  size = read(0,buf,MAX_SIZE);
  if(size <= 0) return NULL;

  /* take just the red part, store it in s and concatenate '\0' */
  char* s = malloc(sizeof(char) * size + 1);
  strncat(s, buf, size);
  strcat(s, "\0");

  return s;
}

char** str_cut (char *input_str, char token, size_t length, int* argc){
  *argc = 0;
  int l = 0;
  char **words = malloc(sizeof(char*));
  if(words == NULL) return NULL;

  /* we go through the whole sentence */
  unsigned i = 0;
  while(i < length){
    /* if found the token, or a backspace delimiter */
    if(input_str[i] == token || input_str[i] == '\n'){
      /* allocate space for 1 more word, take it and store it in the array */
      words = realloc(words, (*argc + 1) * sizeof(char*));
      char *w = malloc(sizeof(char) * l + 1);
      if(w == NULL) return NULL;

      memset(w,'\0', sizeof(char) * l + 1);

      strncat(w, input_str + i - l, l);
      strcat(w, "\0");

      words[*argc] = w;

      /* i++ one more time for the token we've just red */
      (*argc)++; i++;
      l = 0;
    }
    l++;i++;
  }

  words = realloc(words, (*argc + 1) * sizeof(char*));

  return words;
}

int execute_cmd(char **argv){
  int found, w;

  int r = fork();
  /* exit option */
  switch(r){
    case -1 : return -1;
    case 0 :
    found = execvp(argv[0], argv);
    if(found < 0) return -1;
    return 1;
    default :
    wait(&w); break;
  }

  return 1;
}

int execute_tar_cmd(char **argv){
  int found, w;

  char* new_argv0 = malloc(sizeof(char) * (strlen(argv[0]) + 2 + 1));
  memset(new_argv0,'\0',(strlen(argv[0]) + 7 + 1));
  strcat(new_argv0,"cmds/./");
  strcat(new_argv0,argv[0]);
  argv[0] = new_argv0;

  int r = fork();
  /* exit option */
  switch(r){
    case -1 : return -1;
    case 0 :
    found = execvp(argv[0], argv);
    if(found < 0) return -1;
    return 1;
    default :
    wait(&w); break;
  }

  return 1;
}

int one_of_args_is_tar(char **argv, int argc){
  for(int i = 0; i < argc; i++){
    if(strstr(argv[i],".tar/") != NULL) return 1;
  }
  return 0;
}
