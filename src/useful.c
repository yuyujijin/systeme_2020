#include "useful.h"

#define MAX_SIZE 256

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
    free(words[i]);
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

void freeSpecialPath(struct special_path sp){
  // On ne free que sp.path, tout les reste est issue d'un strtok
  if(sp.path != NULL) free(sp.path);
}

char *getRealPath(char *path){
  char pwd[MAX_SIZE];
  memset(pwd,0,MAX_SIZE);
  getcwd(pwd,MAX_SIZE);
  char *tarname = getenv("TARNAME");
  char *tarpath = getenv("TARPATH");
  char *s = malloc(strlen(pwd) + strlen(tarname) + strlen(tarpath) + strlen(path) + 4);
  if(s == NULL) return NULL;
  memset(s,0,strlen(pwd) + strlen(tarname) + strlen(tarpath) + strlen(path) + 4);
  strcat(s,pwd);
  if(strlen(tarname) > 0) strcat(s,"/");
  strcat(s,tarname);
  if(strlen(tarpath) > 0) strcat(s,"/");
  strcat(s,tarpath);
  strcat(s,"/");
  strcat(s,path);
  char *simplified = path_simplifier(s);
  free(s);
  return simplified;
}



struct special_path special_path_maker(char* path){
  /* words will act as a lifo structure */
  char *words[64];
  int size = 0, pathsize = 0;

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
    w = strtok ( NULL, "/");
  }

  /* now we're going to create the 3 requiered strings */
  /* we're going to look through every words of the stack, and act like this :
    - if the words[i] is a tarball (contains ".tar"), then we put it in tar_name and say that tar is true
    - if tar is false, then add words[i] to new_path
    - else, add words[i] to tar_path
  */
  char *new_path = "",*tar_path = "",*tar_name = "";
  int tar = 0, index_path = 0, index_tar = 0;
  for(int i = 0; i < size; i++){
    /* if we didnt went further than the tar (if there is one) */
    if(!tar){
      /* if words[i] is a tar */
      if(strstr(words[i],".tar") != NULL){
        tar_name = malloc(strlen(words[i]) + 1);
        memset(tar_name,'\0',strlen(words[i]) + 1);
        strcpy(tar_name, words[i]);
        strcat(tar_name,"\0");
        tar = 1; continue;
      }
      /* else we add the word to path */
      int length = strlen(new_path);
      if(length == 0){
        new_path = malloc(sizeof(char) * (strlen(words[i]) + 2));
      }else{
        new_path = realloc(new_path, length + strlen(words[i]) + 2);
      }
      memset(new_path + length,'\0',strlen(words[i]) + 2);
      strcat(new_path, words[i]);
      strcat(new_path,"/");
      strcat(new_path,"\0");
      index_path++;
    }else{
      /* same as before, but in tar_path */
      int length = strlen(tar_path);
      if(length == 0){
        tar_path = malloc(sizeof(char) * (strlen(words[i]) + 2));
      }else{
        tar_path = realloc(tar_path, length + strlen(words[i]) + 2);
      }
      memset(tar_path + length,'\0',strlen(words[i]) + 2);
      strcat(tar_path, words[i]);
      strcat(tar_path,"/");
      strcat(tar_path,"\0");
      index_tar++;
    }
    free(words[i]);
  }
  /* and we just create the struct */
  special_path sp = { new_path, tar_path, tar_name };
  return sp;
}
