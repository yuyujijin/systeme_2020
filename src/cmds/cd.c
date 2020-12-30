#include "cd.h"

int cd(char *path){
  char *pathcpy = strdup(path);
  if(pathcpy[strlen(pathcpy) - 1] == '/') pathcpy[strlen(pathcpy) - 1] = '\0';

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

  if(strlen(sp.path) > 0 && chdir(sp.path) < 0){ return -1; }

  if(strlen(sp.tar_name) > 0){
    int fd = open(sp.tar_name,O_RDONLY);
    if(fd < 0){ return -1; }
    close(fd);

    if(strlen(sp.tar_path) > 0 && !exists(sp.tar_name,sp.tar_path)){ return -1; }
  }

  setenv("TARNAME",sp.tar_name,1);
  setenv("TARPATH",sp.tar_path,1);
  
  return 1;
}

char* get_full_path(char *path, char *tar_path){
  if(strlen(tar_path) == 0) return path;
  char *full_path = malloc(strlen(path) + strlen(tar_path) + 3);
  memset(full_path, '\0', strlen(path) + strlen(tar_path) + 3);
  strcat(full_path,tar_path);
  if(tar_path[strlen(tar_path) - 1] != '/') strcat(full_path,"/");
  strcat(full_path,path);
  strcat(full_path,"\0");
  free(path);
  return full_path;
}
