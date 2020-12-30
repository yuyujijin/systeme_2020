#include "cat.h"
int cat(char *const argv[],int argc){
  if(argc == 1) execlp("cat","cat",NULL);
  for(int i = 1; i < argc; i++){
    char *p = getRealPath(argv[i]);
    special_path sp = special_path_maker(p);
    if(strlen(sp.tar_path) > 0) sp.tar_path[strlen(sp.tar_path) - 1] = '\0';

    /* le tar a ouvrir est a l'adresse "/" + pwd + nom du tar */
    char tarlocation[strlen(sp.path) + strlen(sp.tar_name) + 2];
    memset(tarlocation,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
    sprintf(tarlocation,"/%s%s",sp.path,sp.tar_name);

    /* si on est dans un tar */
    if(strlen(sp.tar_name) > 0){
      rdTar(tarlocation,sp.tar_path);
    }else{
      int w;
      tarlocation[strlen(tarlocation) - 1] = '\0';
      switch(fork()){
        case -1 : return -1;
        case 0 : execlp("cat","cat",tarlocation, NULL); exit(0);
        default : wait(&w); break;
      }
    }
  }

  return 1;
}

int main(int argc,char *const argv[]){
  return cat(argv,argc);
}
