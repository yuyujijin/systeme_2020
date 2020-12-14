#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include "cmds/cd.h"

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
int execute_cmd(char **argv);

/* same w/ tar */
int execute_tar_cmd(char **argv,int argc);

void printcwd();

int execute_pipe_cmd(char ***pipelines_args, int argc);

/*
  modify argv[i] if we are inside a tar. if we did cd a.tar/b and then ls c
  --> tar_path=a.tar/b && the commands that will be executed will be ls a.tar/b/c
  since we added the TARPATH in the arguments
*/
int add_tar_path_to_args(char **argv,int argc);

/* read_line read the next line from stdin */
char* read_line();

/* this function check if one of the args is looking INSIDE a tar */
int one_of_args_is_tar(char **argv, int argc);

int main(){
  char* line;

  /* environnement variable that store the additional path */
  setenv("TARPATH","",1);

  while(1){
    printcwd();

    /* read next line */
    line = read_line();
    if(line == NULL) break;

    if(strcmp(line,"\n") == 0){
      free(line);
      continue;
    }

    /* first, we cut our line using '|' as a delimiter */
    int argc;
    char **pipelines = str_cut(line,"|",&argc);

    free(line);
    /* argc-- is because our arrays of words all finish with (null) and we dont
    need it for this array */
    argc--;
    /* we create an array for every sub sequences between the '|' delimiter */
    char **pipelines_args[argc];
    /* and then we recut each sub sequences with the ' ' delimiter */
    for(int i = 0; i < argc; i++){
      if(pipelines[i] != NULL){
        int taille = 0;
        pipelines_args[i] = str_cut(pipelines[i]," ",&taille);
      }else{
        pipelines_args[i] = NULL;
      }
    }

    /* on ne free que le tableau : les cases proviennent de strtok */
    free(pipelines);

    /* on entre exit en toute première commande */
    if(strcmp(pipelines_args[0][0],"exit") == 0) break;

    /* specific case for cd, because we do not execute it */
    if(strcmp(pipelines_args[0][0],"cd") == 0){
      if(pipelines_args[0][1] == NULL) continue;
      if(cd(pipelines_args[0][1]) < 0) perror("cd");
      continue;
    }

    execute_pipe_cmd(pipelines_args,argc);

    /* on libère les pipelines_args */
    for(int i = 0; i < argc; i++) free(pipelines_args[i]);
  }
  char *s = "\nCe fût un plaisir.\n";
   write(STDIN_FILENO,s,strlen(s));
   return 0;
}

int execute_pipe_cmd(char ***pipelines_args, int argc){
  int nbr = argc;

  int pipefds[nbr][2];

  for(int i = 0; i < nbr; i++) pipe(pipefds[i]);

  int w;
  for(int i = 0; i < nbr; i++){
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

      execvp(pipelines_args[i][0],pipelines_args[i]);

      char s[MAX_SIZE];
      memset(s,0,MAX_SIZE);
      sprintf(s,"%s : commande introuvable\n",pipelines_args[i][0]);

      dup2(oldstdin,STDIN_FILENO);
      write(STDIN_FILENO,s,strlen(s));
      exit(-1);
      default : close(pipefds[i][1]); close(pipefds[i-1][0]); waitpid(r,&w,0); break;
    }
  }

  for(int i = 0; i < nbr; i++){ close(pipefds[i][0]); close(pipefds[i][1]); }
  return 1;
}

void printcwd(){
  char bgnline[MAX_SIZE];
  char *cwd = getcwd(NULL, 0);
  cwd = realloc(cwd,strlen(cwd) + strlen(getenv("TARPATH")) + 2);
  strcat(cwd,"/");
  strcat(cwd,getenv("TARPATH"));
  sprintf(bgnline,"%s%s%s:%s%s%s$ ",BOLDGREEN,getlogin(),RESET,BOLDBLUE,cwd,RESET);

  write(STDIN_FILENO, bgnline, strlen(bgnline));

  free(cwd);
}

char *read_line(){
  char buf[MAX_SIZE];
  memset(buf,'\0',MAX_SIZE);
  if(read(STDIN_FILENO, buf, MAX_SIZE) <= 0) return NULL;

  char *s = malloc(sizeof(char) * (strlen(buf)));
  memset(s,'\0',strlen(buf));
  strcpy(s,buf);

  return s;
}

char** str_cut(char *input_str, char *tokens, int* argc){
  //printf("je coupe : _%s_ avec le token : _%s_\n",input_str,tokens);
  *argc = 0;
  char **words = malloc(sizeof(char*));
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

int add_tar_path_to_args(char **argv,int argc){
  char *tar_path=getenv("TARPATH");
  if(argc==1){//if we just say "ls" we gotta show what's inside the tarpath and not pdw
    argc+=1;
    char *cmd=argv[0];
    if(argv==NULL)return -1;
    argv[0]=cmd;
    argv[1]=tar_path;
    return 0;
  }
  for(int i=1;i<argc;i++){
    if(argv[i][0]!='-'){//we don't take option only path
      char *tar_arg=malloc(strlen(argv[i])+strlen(tar_path)+2);
      if(tar_arg==NULL)return -1;
      strcpy(tar_arg,tar_path);
      if(argv[i][0]!='/')strcat(tar_arg,"/");
      strcat(tar_arg,argv[i]);
      argv[i]=tar_arg;
    }
  }
  return 0;
}


int execute_cmd(char **argv){
  execvp(argv[0], argv);
  exit(0);
}

int execute_tar_cmd(char **argv,int argc){
  int found, w;
  char* new_argv0 = malloc(sizeof(char) * (strlen(argv[0]) + 2 + 1));
  memset(new_argv0,'\0',(strlen(argv[0]) + 7 + 1));
  strcat(new_argv0,"cmds/./");
  strcat(new_argv0,argv[0]);
  argv[0] = new_argv0;
  if(strstr(getenv("TARPATH"),".tar") != NULL){
    if(add_tar_path_to_args(argv,argc)<0)return -1;
  }
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

  free(new_argv0);

  return 1;
}

int one_of_args_is_tar(char **argv, int argc){
  for(int i = 0; i < argc; i++){
    if(strstr(argv[i],".tar/") != NULL) return 1;
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
