#include "useful.h"

char* path_simplifier(char* path){
  /* words will act as a lifo structure */
  char *words[64];
  int size = 0, pathsize = 0, len = 0;

  char *w = strtok(path,"/");

  while ( w != NULL ) {
    /* if its ".", we do nothing */
    if(strcmp(w,".") == 0){ w = strtok(NULL,"/"); continue; }
    /*
    if its ".." and :
      (1) stack is empty -> we do nothing
      (2) stack is not empty ->
        - if top is ".." then we do nothing
        - else we pop
    */
    if(strcmp(w,"..") == 0){
      if(size > 0 && strcmp(words[size - 1],"..") != 0){
        size--; pathsize -= strlen(words[size]) + 1; w = strtok(NULL, "/");
        continue;
      }
    }
    /* if its not "." nor ".." (or we didnt pushed ".."), we just add the word to the stack */
    char *wc = malloc(strlen(w) + 1);
    memset(wc,'\0',strlen(w) + 1);
    strcpy(wc,w);
    strcat(wc,"\0");

    pathsize += strlen(wc) + 1;
    words[size++] = wc;
    len += strlen(wc);
    w = strtok ( NULL, "/");
  }

  if(size == 0) return ".";

  char *s = malloc(sizeof(char) * (len + size));
  if(s == NULL) return NULL;
  memset(s,0,len + size);

  for(int i = 0; i < size; i++){
    strcat(s,words[i]);
    if(i < size - 1) strcat(s,"/");
  }

  return s;
}

char* pathminus(char *path, char *lastarg){
  char *s = malloc((strlen(path) - strlen(lastarg) + 1) * sizeof(char));
  memset(s,'\0',strlen(path) - strlen(lastarg) + 1);
  strncat(s,path,strlen(path) - strlen(lastarg));
  strcat(s,"\0");
  
  return s;
}

char *getLastArg(char *path){
  return (strrchr(path,'/') != NULL)? strrchr(path,'/') + 1: path;
}
