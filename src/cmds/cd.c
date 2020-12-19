#include "cd.h"

int cd(char *path){
  char *pathcpy = strdup(path);

  char *tar_name = getenv("TARNAME");
  char *tar_path = getenv("TARPATH");
  char *fp = get_full_path(get_full_path(pathcpy,tar_path),tar_name);
  special_path sp = special_path_maker(fp);
  free(fp);
  /*
  now we have just to check if :
    - sp.tar_path contains ".tar" -> error (no tar in tar)
    - sp.path + sp.tar_name is a tar and contains sp.tar_path
    - sp.path is accessible
  if every of the above is ok, we access sp.path, modify the two env variable TARNAME and TARPATH with our values
  */
  if(strstr(sp.tar_path,".tar") != NULL){ errno = ENOTDIR; return -1; }

  char tarball_path[strlen(sp.path) + strlen(sp.tar_name) + 1];
  memset(tarball_path,'\0',strlen(sp.path) + strlen(sp.tar_name) + 1);
  strcat(tarball_path,sp.path); strcat(tarball_path,sp.tar_name); strcat(tarball_path,"\0");
  if(strlen(sp.tar_name) > 0 && !exists(tarball_path, sp.tar_path)){ errno = ENOENT;  return -1; }

  if(strlen(sp.path) > 0 && chdir(sp.path) < 0) return -1;
  setenv("TARNAME",sp.tar_name,1);
  setenv("TARPATH",sp.tar_path,1);

  return 0;
}

char* get_full_path(char *path, char *tar_path){
  if(strlen(tar_path) == 0) return path;
  char *full_path = malloc(strlen(path) + strlen(tar_path) + 1);
  memset(full_path, '\0', strlen(path) + strlen(tar_path) + 2);
  strcat(full_path,tar_path);
  strcat(full_path,"/");
  strcat(full_path,path);
  strcat(full_path,"\0");
  return full_path;
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
