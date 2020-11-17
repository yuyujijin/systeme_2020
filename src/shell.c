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
int execute_cmd(char **argv,int argc);

/* same w/ tar */
int execute_tar_cmd(char **argv,int argc);

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
int one_of_args_is_tar(char **argv, int argc);

/* let's say we have "cat a > b > c" then we fill int *out with
  out[0] --> index of first occurence of '>'
  out[1] -->  index of first argument after '>'
  in our case we would have out={2,5}
  We return -1 if any error occured
  We also create a blank file(O_CREAT|O_TRUNC) each we pass an arguments after the first '>'
  to duplicate this execution
*/
int redirection(char **argv,int argc,int *out,int *in);

/* this function returns new_argv for redirection so if we have {"ls","a","b","c",">","d","e","f"} then we return {"ls","a","b","c"}
   return NULL if any error occured
 */
char ** new_argv_redirection(char **argv,int argc);



int main(){
  char* line;

  /* environnement variable that store the additional path */
  setenv("TARPATH","",1);

  while(1){
    printcwd();

    /* read next line */
    line = read_line();
    if(line == NULL) break;

    /* cut it in words array with space char delimiter */
    int argc;
    char **args = str_cut(line,' ', strlen(line), &argc);
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
    if(strstr(getenv("TARPATH"),".tar") != NULL
       || one_of_args_is_tar(args + 1, argc -1)){
      if(execute_tar_cmd(args,argc) < 0)
	printf("Commande %s non reconnue\n",args[0]);
    }else{
      if(execute_cmd(args,argc) < 0)
	printf("Commande %s non reconnue\n",args[0]);
    }

    for(int i = 0; i < argc; i++){
      free(args[i]);
    }
    free(args);
  }

  write(STDIN_FILENO,"\n",2);
  return 0;
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
  read(STDIN_FILENO, buf, MAX_SIZE);

  char *s = malloc(sizeof(char) * (strlen(buf)));
  memset(s,'\0',strlen(buf));
  strcpy(s,buf);

  return s;
}

char** str_cut(char *input_str, char token, size_t length, int* argc){
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
      char *w = malloc(sizeof(char) * l + 2);
      if(w == NULL) return NULL;

      memset(w,'\0', sizeof(char) * l + 2);

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
  words[*argc] = NULL;

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


int redirection(char **argv,int argc,int *out,int *in){//let's say we have "cat a > b > c" redirection_out returns the {2,5} as argv[5]="c" and argv[2]=">" so we can deduce which argument to execute and which to redirect
  int out_index=-1;//first argument after last occurence of '>'
  int out_first_index=-1;//first occurence of '>'
  int in_index=-1;//first argument after last occurence of '<'
  int in_first_index=-1;//first occurence of '<'
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],">")==0){//we don't break the loop cause we want the last redirection: cat a>b>c ---> "c"
      if(out_first_index==-1)out_first_index=i;//we only want the first ">"
      out_index=i;
      if(i<argc-1){//just to make sure we're not out of bounds
        int fd=open(argv[i+1],O_WRONLY|O_CREAT|O_TRUNC,0644);//we create the file here but don't do the dup2 yet cause we only want the redirection to be on the last ">"
        if(fd<0){
          perror(argv[i]);
          return -1;
        }
        close(fd);
      }
    }
    if(strcmp(argv[i],"<")==0){//we don't break the loop cause we want the last redirection: cat a < b < c ---> "c"
      if(in_first_index==-1)in_first_index=i;//we only want the first "<" for first_index
      in_index=i;
    }
  }
  in[0]=in_first_index;
  in[1]=in_index+1;
  out[0]=out_first_index;
  out[1]=out_index+1;
  return in_index>0||out_index>0;
}

char ** new_argv_redirection(char **argv,int argc){
  char **new_argv=NULL;
  int is_in=0;
  int is_out=0;
  int index_new_argv=0;
  for(int i=0;i<argc;i++){
    if(argv[i][0]=='>')is_out=1;//this argv[i] and argv[i+1] are not gonna be in new_argv
    if(argv[i][0]=='<')is_in=1;
    if(is_out==0&&is_in==0){
      new_argv=realloc(new_argv,sizeof(char*)*(index_new_argv+1));
      if(new_argv==NULL)return NULL;
      new_argv[index_new_argv]=malloc(strlen(argv[i])+1);
      if(new_argv==NULL)return NULL;
      memcpy(new_argv[index_new_argv],argv[i],strlen(argv[i]));
      index_new_argv+=1;
    }
    if(is_out==1||is_in==1){//this let us skip the next argument aswell
      i+=1;
      is_out=0;
      is_in=0;
    }
  }
  new_argv=realloc(new_argv,sizeof(char*)*(index_new_argv+1));
  new_argv[index_new_argv]=NULL;
  return new_argv;
}


int execute_cmd(char **argv,int argc){
  int w;
  int *out=malloc(2*sizeof(int));
  int *in=malloc(2*sizeof(int));
  /* exit option */
  switch(fork()){
    case -1 : return -1;
    case 0 :
      if(redirection(argv,argc,out,in)<0)exit(EXIT_FAILURE);
      if(out[1]>0||in[1]>0){//we have a redirection in or out doesn't matter yet
        char **new_argv=new_argv_redirection(argv,argc);
        if(new_argv==NULL)exit(EXIT_FAILURE);
        if(out[1]>0){//if there is an output redirection then we open the file and redirect with dup2
          int fd_out=open(argv[out[1]],O_WRONLY|O_CREAT|O_TRUNC,0644);
          if(fd_out<0)exit(EXIT_FAILURE);
          dup2(fd_out,1);
          close(fd_out);
        }
        if(in[1]>0&&new_argv[1]==NULL){//if there is an input redirecion and we only do the commande without any arguments then we open the file and redirect with dup2
          //cat a < b > c ---> cat a > c (b is not used). That's why we check if new_argv[1] is equal to NULL
          int fd_in=open(argv[in[1]],O_RDONLY);
          if(fd_in<0)exit(EXIT_FAILURE);
          dup2(fd_in,0);
          close(fd_in);
        }
        if(execvp(new_argv[0],new_argv)<0)exit(EXIT_FAILURE);
        exit(EXIT_SUCCESS);
      }
      else{
        if(execvp(argv[0],argv)<0)exit(EXIT_FAILURE);
      }
      exit(EXIT_SUCCESS);
    default :
    wait(&w); break;
  }

  return 1;
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
