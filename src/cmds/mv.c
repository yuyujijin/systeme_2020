#include "mv.h"
int main(int argc, char **argv) {
  return mv(argc,argv);
}

int oneOfArgsIsTar(int argc, char **argv){
  for(int i = 1; i < argc; i++){
    char *duplic = strdup(argv[i]);
    if(strstr(getRealPath(duplic),".tar") != NULL){ free(duplic); return 1; }
    free(duplic);
  }
  return 0;
}

int exec_cmd(char *cmd, char **argv){
  char *pathname = getenv("TARCMDSPATH");

  char pathpluscmd[strlen(pathname) + 1 + strlen(cmd)];
  memset(pathpluscmd,0,strlen(pathname) + 1 + strlen(cmd));
  sprintf(pathpluscmd,"%s/%s",pathname,cmd);

  execv(pathpluscmd,argv);
  return -1;
}

int mv(int argc, char **argv){
  if(argc==1){
    if(write(2,"mv: opérande de fichier manquant",strlen("mv: opérande de fichier manquant"))<0)return -1;
    if(write(2,". Saisissez « mv --help » pour plus d'informations.",strlen("Saisissez « mv --help » pour plus d'informations."))<0)return -1;
    if(write(2,argv[1],strlen(argv[1]))<0)return -1;
    return -1;
  }
  if(argc==2){
    if(write(2,"opérande de fichier cible manquant après '",strlen("opérande de fichier cible manquant après '"))<0)return -1;
    if(write(2,argv[1],strlen(argv[1]))<0)return -1;
    if(write(2,"'\n Saisissez « mv --help » pour plus d'informations.\n",strlen("'\n Saisissez « mv --help » pour plus d'informations.\n"))<0)return -1;
    return -1;
  }
  /* Dans le cas ou aucun des arguments ne traite en fait un tar */
  if(oneOfArgsIsTar(argc, argv) == 0){
    for(int i = 1; i < argc ; i++){
      char *s = getRealPath(argv[i]);
      struct special_path sp = special_path_maker(s);
      free(s);

      char *chemin = malloc(sizeof(char) * (strlen(sp.path) + 2));
      memset(chemin,0,strlen(sp.path) + 2);
      sprintf(chemin,"/%s",sp.path);
      chemin[strlen(chemin) - 1] = '\0';

      argv[i] = chemin;
    }
    execvp("mv",argv);
    exit(-1);
  }

  char *new_argv[argc+1];
  new_argv[0]=argv[0];
  new_argv[1]="-r";
  for(int i=1;i<argc;i++){
    new_argv[i+1]=argv[i];
  }
  new_argv[argc+1]=NULL;

  // Sinon
  int r = fork(), w;
  // On copie en premier
  switch (r) {
    case -1 : perror("fork"); exit(EXIT_FAILURE);
    case 0  :
      exec_cmd("cp",new_argv);
      exit(1);
    default : waitpid(r, &w, 0); break;
  }
  if(WEXITSTATUS(w)< 0) return -1;

  // Puis on supprime
  r = fork();
  switch (r) {
    case -1 : perror("fork"); exit(EXIT_FAILURE);
    case 0  :
      new_argv[argc] =NULL;
      exec_cmd("rm",new_argv);
      exit(1);
    default : waitpid(r, &w, 0); break;
  }
  if(WEXITSTATUS(w) < 0) return -1;
  return 1;
}
