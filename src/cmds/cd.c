#include "cd.h"

/* take path (TARPATH), and modify tarname and newpath */
int truncate_tarname_path(char *path, char **tarname, char **newpath);

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
  char* elem = strtok(path,"/");
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
    char* tarname;
    char* newpath;

    /* we clone tarpath because strtok (in truncate_...) modifies the path */
    char* clonetarpath = malloc((strlen(getenv("TARPATH")) + 1) * sizeof(char));
    if(clonetarpath == NULL) return -1;
    strcat(clonetarpath, getenv("TARPATH"));
    strcat(clonetarpath,"\0");

    /* we get the name of the tar we're in + rest of path */
    truncate_tarname_path(clonetarpath, &tarname, &newpath);

    /* megapath is just tarpath + "/" + elem + "/" */
    char* megapath;
    if(strlen(newpath) > 0){
      megapath = malloc(sizeof(char) * (strlen(newpath) + strlen(elem) + 1));
      if(megapath == NULL) return -1;
      strcat(megapath,newpath);
      strcat(megapath,"/");
    }else{
      megapath = malloc(sizeof(char) * (strlen(elem) + 1));
      if(megapath == NULL) return -1;
    }
    strcat(megapath,elem);
    strcat(megapath,"/");

    /* check if this file exist in tarball */
    if(exists(tarname, megapath) == 1){
      /* if it is, we just update the env variabe TARPATH */
      char* tarpath = getenv("TARPATH");
      char *newtarpath = malloc((strlen(tarpath) + strlen(elem) + 2) * sizeof(char));
      if(newtarpath == NULL) return -1;
      memset(newtarpath,'\0',strlen(tarpath) + strlen(elem) + 2);

      strcat(newtarpath,tarpath);
      strcat(newtarpath,"/");
      strcat(newtarpath,elem);

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
    return cd_aux(path + strlen(elem) + 1);
  }

  /* CASE 2.2 : normal case */
  if(chdir(elem) < 0) return -1;
  return cd_aux(path + strlen(elem) + 1);
}

int truncate_tarname_path(char *path, char **tarname, char **newpath){
  *tarname = strtok(path,"/");
  if(*tarname == NULL) return -1;

  *newpath = malloc((strlen(path) - strlen(*tarname)));
  if(*newpath == NULL) return -1;

  memset(*newpath,'\0',strlen(path) - strlen(*tarname));
  strcat(*newpath, path + strlen(*tarname) + 1);

  return 1;
}