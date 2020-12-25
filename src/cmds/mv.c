#include "mv.h"
int main(int argc, char const *argv[]) {
  return mv(argc,argv);
}

int mv(int argc, char const *argv[]){
  int w;
  if(argc==1){
    if(write(1,"mv: opérande de fichier manquant",strlen("mv: opérande de fichier manquant"))<0)return -1;
    if(write(1,"Saisissez « mv --help » pour plus d'informations.",strlen("Saisissez « mv --help » pour plus d'informations."))<0)return -1;
    if(write(1,argv[1],strlen(argv[1]))<0)return -1;
    return -1;
  }
  if(argc==2){
    if(write(1,"opérande de fichier cible manquant après '",strlen("opérande de fichier cible manquant après "))<0)return -1;
    if(write(1,argv[1],strlen(argv[1]))<0)return -1;
    if(write(1,"Saisissez « mv --help » pour plus d'informations.",strlen("Saisissez « mv --help » pour plus d'informations."))<0)return -1;
    return -1;
  }
  switch (fork()) {
    case -1 : perror("fork");exit(EXIT_FAILURE);
    case 0  :
    {
      char *pathname = getenv("TARCMDSPATH");
      /* We execute cp */
      char pathpluscmdCp[strlen(pathname) + 1 + strlen("cp")];
      memset(pathpluscmdCp,0,strlen(pathname) + 1 + strlen("cp"));
      sprintf(pathpluscmdCp,"%s/%s",pathname,"cp");
      if(execv(pathpluscmdCp,(char * const*)argv)<0){
        perror(argv[0]);
        exit(-1);
      }
      /*and now rm all the old file except the destination*/
      for(int i=1;i<argc-1;i++){
        memset(pathpluscmdCp,0,strlen(pathname) + 1 + strlen("rm"));
        sprintf(pathpluscmdCp,"%s/%s",pathname,"rm");
        switch (fork()) {
          case -1 : perror("fork");exit(EXIT_FAILURE);
          case 0 :
            /* We execute rm on argv[i] */
            if(execl(pathpluscmdCp,argv[i],NULL)<0){
              perror(argv[i]);
              exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
          default : wait(&w);if(WEXITSTATUS(w)==EXIT_FAILURE)exit(-1);exit(EXIT_SUCCESS);
        }
      }
      exit(EXIT_SUCCESS);
    }
    default : wait(&w);if(WEXITSTATUS(w)==EXIT_FAILURE)return -1;return argc;
  }
}
