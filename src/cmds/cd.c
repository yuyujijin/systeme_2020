#include "cd.h"

int cd(char *path){
  if(strcmp(path,"~") == 0){ setenv("TARPATH", "", 1); return chdir(getenv("HOME")); }
  char* pwd = getcwd(NULL, 0);
  char* tarpwd = getenv("TARPATH");
  /* first we try if we can access it */
  if(cd_aux(path) >= 0) return 1;
  /* if not, cancel every changes */
  chdir(pwd);
  setenv("TARPATH",tarpwd,1 );
  return -1;
}

int cd_aux(char *path){
  /* we 'empty' the whole path */
  if(strlen(path) <= 0 || strcmp(path,"/") == 0) return 1;

  /* get first element of path */
  char pathcpy[strlen(path)];
  memset(pathcpy,'\0',strlen(path));
  strcpy(pathcpy,path);

  char* elem = strtok(pathcpy,"/");
  if(elem == NULL) return -1;

  /* CASE 1 : WE'RE IN A TAR */
  if(strstr(getenv("TARPATH"),".tar") != NULL){

    /* CASE 1.1 : IS IT A TAR IN A TAR (FORBIDDEN!) */
    if(strstr(elem,".tar") != NULL){ errno = EADDRNOTAVAIL; return -1; }

    /* CASE 1.2 : "." (SAME DIR) */
    /* we recall and skip elem + '/' */
    if(strcmp(elem,".") == 0) return cd_aux(path + strlen(elem) + 1);
		/* CASE 1.3 : ".." (PREV DIR) */
    if(strcmp(elem,"..") == 0){
      /* newpath will be path without the last dirname */
      char newpath[strlen(getenv("TARPATH"))];
      strcat(newpath, getenv("TARPATH"));
      while(strlen(newpath) > 0 && newpath[strlen(newpath) - 1] != '/'){
        newpath[strlen(newpath) - 1] = '\0';
      }
      newpath[strlen(newpath) - 1] = '\0';
      setenv("TARPATH",newpath,1);

      return cd_aux(path + strlen(elem) + 1);
    }

	  /* CASE 1.4 : ANY DIRECTORY */
    char tarpathcpy[strlen(getenv("TARPATH"))];
    strcat(tarpathcpy, getenv("TARPATH"));

    char *tarname = strtok(tarpathcpy,"/");

    int size = strlen(elem) + 1;
    if(strlen(getenv("TARPATH")) > strlen(tarname)) size+= strlen(tarname) + 1;

    /* megapath is just tarpath + "/" + elem + "/" */
    char megapath[size];
    memset(megapath,'\0',size);

    if(strlen(getenv("TARPATH")) > strlen(tarname)){ strcat(megapath,getenv("TARPATH") + strlen(tarname) + 1); strcat(megapath,"/");  }
    strcat(megapath,elem);
    strcat(megapath,"/");

    /* check if this file exist in tarball */
    if(exists(tarname, megapath) == 1){
      /* if it is, we just update the env variabe TARPATH */
      char newtarpath[strlen(tarname) + 1 + strlen(megapath)];
      memset(newtarpath,'\0',strlen(tarname) + 1 + strlen(megapath));
      megapath[strlen(megapath) - 1] = '\0';

      strcat(newtarpath,tarname);
      strcat(newtarpath,"/");
      strcat(newtarpath,megapath);

      setenv("TARPATH", newtarpath, 1);

      return cd_aux(path + strlen(elem) + 1);
    }

		/* if not, we just return that we cant */
    errno = ENOENT;
    return -1;
  }
  /* CASE 2 :  WE'RE NOT IN TARBALL */

  /* CASE 2.1 : WE'RE ENTERING A TAR */
  if(strstr(elem,".tar") != NULL){
    /* if its a FAKE tar */
    if(!isTar(elem)){ errno = ENOENT; return -1; }

    char* tarpath = getenv("TARPATH");
    if(tarpath == NULL) return -1;

    char *newpath = malloc((strlen(tarpath) + strlen(elem) + 2) * sizeof(char));
    if(newpath == NULL) return -1;
    memset(newpath,'\0',strlen(tarpath) + strlen(elem) + 2);

    strcat(newpath,elem);
    setenv("TARPATH", newpath,1);

    free(newpath);

    return cd_aux(path + strlen(elem) + 1);
  }

  /* CASE 2.2 : normal case */
  if(chdir(elem) < 0) return -1;
  return cd_aux(path + strlen(elem) + 1);
}
