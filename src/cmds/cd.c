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
  /*check if we're in a tar */
  if(strstr(getenv("TARPATH"),".tar") != NULL){
		/* get first element of path */
    char* elem = strtok(path,"/");
    if(elem == NULL) return -1;

    /* no tar in a tar */
    if(strstr(elem,".tar") != NULL) return -1;

		/* same dir */
    if(strcmp(elem,".") == 0) return cd_aux(path + strlen(elem) + 1);
		/* prev dir */
    if(strcmp(elem,"..") == 0){
      char newpath[strlen(getenv("TARPATH"))];
      strcat(newpath, getenv("TARPATH"));
      while(newpath[strlen(newpath) - 1] != '/'){
        newpath[strlen(newpath) - 1] = '\0';
      }
      newpath[strlen(newpath) - 1] = '\0';
      setenv("TARPATH",newpath,1);
      return cd_aux(path + strlen(elem) + 1);
    }
		/* any dir */
		/* TODO
		get the tarball's name we're in, get the rest of the path and concatenate it with elem
		check if this path exist in the tarball and if its a directory
		if not, return -1;
		if it is, add elem to the tarpath and recall on path + strlen(elem) + 1
		*/
    return 0;
  }
  /* if we're not in a tarball */
  /* we take the first elem in path, enter it and recall on the rest */
  char* elem = strtok(path,"/");
  if(elem == NULL) return -1;
  /* if we're trying to enter a tar */
  if(strstr(elem,".tar") != NULL){
    /* if its a FAKE tar */
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
  /* if we're not trying to access a tarball */
  if(chdir(elem) < 0) return -1;
  return cd_aux(path + strlen(elem) + 1);
}
