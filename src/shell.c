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
int redirection_out(char **argv,int argc,int *out){

/* this function fill int* option with the index of all the option inside argv,
   so if we have {"ls","a","-i",">","-a","b"} then option={2,4}
   we return the number of option, if any error occured we return -1
 */
int get_option_index(char **argv,int argc,int *option)

/* this function returns new_argv for redirection so if we have {"ls","a","b","c",">","d","e","f"} then we return {"ls","a","b","c"}
   return NULL if any error occured
 */
char ** new_argv_redirection(char **argv,int argc,int* out)



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


int redirection_out(char **argv,int argc,int *out){//let's say we have "cat a > b > c" redirection_out returns the {2,5} as argv[5]="c" and argv[2]=">" so we can deduce which argument to execute and which to redirect
  int index=-1;
  int first_index=-1;
  for(int i=1;i<argc;i++){
      if(strcmp(argv[i],">")==0){//we don't break the loop cause we want the last redirection: cat a>b>c ---> "c"
        if(first_index==-1)first_index=i;//we only want the first ">"
        index=i;
        if(i<argc-1){//this verification is in the case of "cat a > ": this is an error situation that we must check
          int fd=open(argv[i+1],O_RDWR|O_CREAT|O_TRUNC,0644);//we create the file here but don't do the dup2 yet cause we only want the redirection to be on the last ">"
          if(fd<0)return -1;
          close(fd);
        }
      }
  }
  out[0]=first_index;
  out[1]=index+1;
  if(index>0)return index+1;
  return 0;
}

int get_option_index(char **argv,int argc,int *option){
  int count=0;
  for(int i=1;i<argc;i++){
    if(argv[i][0]=='-'){//we go with the standard that every option start with '-'
      count+=1;
    }
  }
  if(count==0)return 0;
  option=malloc(sizeof(int)*count);
  if(option==NULL)return -1;
  int index_option=0;
  for(int i=1;i<argc;i++){
    if(argv[i][0]=='-'){//we go with the standard that every option start with '-'
      option[index_option]=i;
      index_option++;
    }
  }
  return count;
}

char ** new_argv_redirection(char **argv,int argc,int* out){
  int *option=NULL;//if we have redirection it's best to know the location of all the option since, we could have ls a > b -l
  int nb_option=get_option_index(argv,argc,option);
  if(nb_option<0)return NULL;
  char **new_argv=malloc(out[0]+nb_option+1);
  if(new_argv==NULL)return NULL;
  int index_new_argv=0;
  for(int i=0;i<out[0];i++){//we first copy all the argument before the first '>'
    new_argv[index_new_argv]=malloc(strlen(argv[i])+1);
    if(new_argv[index_new_argv]==NULL)return NULL;
    strcpy(new_argv[index_new_argv],argv[i]);
    new_argv[index_new_argv][strlen(argv[i])]='\0';
    index_new_argv+=1;
  }
  if(nb_option>0){// we got option: "ls > b -l -p"
    //we need to append to new_argv the option
    for(int i=0;i<nb_option;i++){
      new_argv[index_new_argv]=malloc(strlen(argv[option[i]])+1);
      if(new_argv[index_new_argv]==NULL)return NULL;
      strcpy(new_argv[index_new_argv],argv[option[i]]);
      new_argv[index_new_argv][strlen(argv[option[i]])]='\0';
      index_new_argv+=1;
    }
    //now we have: let's say "ls a -l > b -p > c d -f" then new_argv= {ls,a,-l,-p,-f}
  }
  new_argv[out[0]+nb_option]=NULL;//we must terminate with NULL
  return new_argv;
}
int execute_cmd(char **argv,int argc){
  int w;
  int *out=malloc(2*sizeof(int));
  int r = fork();
  /* exit option */
  switch(r){
    case -1 : return -1;
    case 0 :
      if(redirection_out(argv,argc,out)<0)exit(EXIT_FAILURE);//we got an error while opening the file
      if(out[1]>0){
        char **new_argv=new_argv_redirection(argv,argc,out);
        if(new_argv==NULL)exit(EXIT_FAILURE);
        int fd;
        for(int i=out[1];i<argc;i++){//we do dup2 on every arguments after the last ">". Ex: "cat a > b c" --> we loop on b and c
          int f=fork();
          switch (f) {
            case -1:return -1;
            case  0:
              fd=open(argv[i],O_RDWR|O_CREAT|O_TRUNC,0644);
              if(fd<0)exit(EXIT_FAILURE);
              dup2(fd,1);
              dup2(fd,2);
              if(execvp(new_argv[0],new_argv)<0)exit(EXIT_FAILURE);
              close(fd);
              exit(EXIT_SUCCESS);
              break;
            default :wait(NULL);break;
          }
        }
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
