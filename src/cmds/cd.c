#include "cd.h"
#include "tar_manipulation.c"

int cd(char *path){
  if(strcmp(path,"~") == 0){ setenv("TARPATH", "", 1); return chdir(getenv("HOME")); }
  char* pwd = getcwd(NULL, 0);
  char* tarpwd = getenv("TARPATH");
  /* first we try if we can access it */
  if(cd_aux(path) >= 0) return 0;
  /* if not, cancel every changes */
  chdir(pwd);
  setenv("TARPATH",tarpwd,1 );
  return -1;
}

int cd_aux(char *path){
  if(strcmp(path,"") == 0 || strcmp(path,"/") == 0) return 0;
  /* we're in a tar */
  if(strstr(getenv("TARPATH"),".tar") != NULL){
    /* no tar in a tar */
    if(strstr(path,".tar") != NULL) return -1;

    char* elem = strtok(path,"/");
    if(elem == NULL) return -1;
    if(strcmp(elem,".") == 0) return cd_aux(path + strlen(elem) + 1);
    if(strcmp(elem,"..") == 0){
      char newpath[strlen(getenv("TARPATH"))];
      strcat(newpath, getenv("TARPATH"));
      do{
        newpath[strlen(newpath) - 1] = '\0';
      }while(newpath[strlen(newpath) - 1] != '/');
      setenv("TARPATH",newpath,1);
      return cd_aux(path + strlen(elem) + 1);
    }
    //if(!tar_dir_exist(path)) return -1;
    return 0;
  }
  /* we're not */
  /* we take the first elem in path, enter it and recall on the rest */
  char* elem = strtok(path,"/");
  if(elem == NULL) return -1;
  /* we're trying to enter a tar */
  if(strstr(elem,".tar") != NULL){
    /* a FAKE tar */
    if(!isTar(elem)) return -1;
    char* tarpath = getenv("TARPATH");
    if(tarpath == NULL) return -1;
    char *newpath = malloc((strlen(tarpath) + strlen(elem) + 2) * sizeof(char));

    if(newpath == NULL) return -1;
    memset(newpath,'\0',strlen(newpath) + strlen(elem) + 2);

    strcat(newpath,"/");
    strcat(newpath,elem);
    setenv("TARPATH", newpath,1);
    return cd_aux(path + strlen(elem) + 1);
  }
  /* we're not */
  if(chdir(elem) < 0) return -1;
  return cd_aux(path + strlen(elem) + 1);
}
