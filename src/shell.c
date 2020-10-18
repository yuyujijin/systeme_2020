#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ls.h"
int MAX_SIZE = 256;

/* str_cut takes a input string of size length, and returns a array of string containing
the sub-string of input_str delimited by tokens (number of sub-string is given by arc) */
char** str_cut(char *input_str, char token,size_t length, int* argc);

/* read_line read the next line from stdin */
char* read_line();

/* two placeholder functions */
void cd();
void ls(char *const args[],int argc);

/* number of functions availabe */
int CMDS_NBR = 2;
/* name of functions availabe, both array have similare index for same func */
char *commands[] = {"cd","ls"};
/* array of functions available
~ not really at ease with this syntax right now, found it on here
https://stackoverflow.com/questions/252748/how-can-i-use-an-array-of-function-pointers
i'll inform myself more about it ~ */
void (*commands_func[])() = {&cd,&ls};

int main(){
  size_t size;
  char* line;

  while(1){
    write(1,"$ ",1);

    /* read next line */
    line = read_line();
    if(line == NULL) return -1;

    /* cut it in words array with space char delimiter */
    int argc;
    char **args = str_cut(line,' ', strlen(line), &argc);
    if(args == NULL) return -1;

    /* exit option */
    if(strcmp(args[0],"exit") == 0) exit(0);

    /* compare first args of words array with every function known */
    int found = 0;
    for(int j = 0; j < CMDS_NBR; j++){
      if(strcmp(args[0],commands[j]) == 0){ found = 1; commands_func[j](args, argc); }
    }

    /* in case we didnt find anything */
    if(!found && strlen(args[0]) > 0) printf("Commande %s non reconnue\n",args[0]);

  }
  return 0;
}

char *read_line(){
  char buf[MAX_SIZE];
  size_t size;

  size = read(0,buf,MAX_SIZE);
  if(size < 0) return NULL;

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
  int i = 0;
  while(i < length){
    /* if found the token, or a backspace delimiter */
    if(input_str[i] == token || input_str[i] == '\n'){
      /* allocate space for 1 more word, take it and store it in the array */
      words = realloc(words, (*argc + 1) * sizeof(char*));
      char *w = malloc(sizeof(char) * l + 1);
      if(w == NULL) return NULL;

      strncat(w, input_str + i - l, l);
      strcat(w, "\0");

      words[*argc] = w;

      /* i++ one more time for the token we've just red */
      (*argc)++; i++;
      l = 0;
    }
    l++;i++;
  }

  return words;
}

/* placeholder funcs */
void cd(){ printf("commande tapée : cd\n"); }
void ls(char *const args[],int argc){ls_call(args,argc); }
