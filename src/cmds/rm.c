#include "rm.h"

int main(int argc, char** argv)
{
  if(argc == 1){
    perror("rm : manque 1 argument");
    return -1;
  }
  if(argc == 2 && !strcmp(argv[1],"-r")){
    perror("rm : manque 1 argument");
    return -1;
  }
  return rm_call(argc,argv);
}

//call rm if we're in a regular path
//else calls rm
int rm_call(int argc,char** argv)
{
  int optionR = strcmp(argv[1],"-r") == 0;
  for(int i = 1; i < argc; i++){
    if(!strcmp(argv[i],"-r")) continue;

    char *p = getRealPath(argv[i]);
    special_path sp = special_path_maker(p);
    if(strlen(sp.tar_path) > 0) sp.tar_path[strlen(sp.tar_path) - 1] = '\0';

    /* si on est dans un tar */
    if(strlen(sp.tar_name) > 0){
      /* le tar a ouvrir est a l'adresse "/" + pwd + nom du tar */
      char tarlocation[strlen(sp.path) + strlen(sp.tar_name) + 2];
      memset(tarlocation,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
      sprintf(tarlocation,"/%s%s",sp.path,sp.tar_name);

      struct posix_header *ph = getHeader(tarlocation,sp.tar_path);

      if(strlen(sp.tar_path) == 0){
        if(optionR){
          int r = fork();
          switch(r){
            case -1 : return -1;
            case 0 :
            execlp("rm","rm",tarlocation,NULL);
            exit(-1);
            default : waitpid(r, NULL, 0); break;
          }
          continue;
        }else{
          perror("est un dossier. (vous avez peut être oublié l'option '-r'?)");
          continue;
        }
      }

      if(ph == NULL){
        perror("impossible d'ouvrir le fichier.\n");
        continue;
      }

      if(ph->typeflag == '5' && !optionR){
        perror("est un dossier. (vous avez peut être oublié l'option '-r'?)");
        continue;
      }

      if(rm_tar(tarlocation, sp.tar_path, optionR) < 0) printf(":/\n");

    }else{
      int r = fork();
      char chemin[strlen(sp.path) + 2];
      memset(chemin,0,strlen(sp.path) + 2);
      sprintf(chemin,"/%s",sp.path);
      chemin[strlen(chemin) - 1] = '\0';

      switch(r){
        case -1 : return -1;
        case 0 :
        if(optionR){
          execlp("rm","rm","-r",chemin,NULL);
        }else{
          execlp("rm","rm",chemin,NULL);
        }
        exit(-1);
        default : waitpid(r, NULL, 0); break;
      }
    }
  }
  return 0;
}

int rm_tar(char *tarname, char *tarpath, int optionR)
{
  if(!optionR){
    if(rmTar(tarname,tarpath) < 0) return -1;
    return 1;
  }
  int fd = open(tarname,O_RDONLY);
  if(fd < 0) return -1;
  char s[strlen(tarpath) + 2];
  memset(s,0,strlen(tarpath) + 2);
  sprintf(s,"%s/",tarpath);
  tarpath = s;

  while(1){
    struct posix_header tampon;
    if(read(fd, &tampon, sizeof(struct posix_header)) < 0)
    { perror("erreur lors de  la lecture du tar.\n"); return -1; }

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    /* same prefix */
    if(!strncmp(tampon.name,tarpath,strlen(tarpath))){
      if(rmTar(tarname,tampon.name) < 0) return -1;
      lseek(fd,0,SEEK_SET);
    }else{
      unsigned int filesize;
      sscanf(tampon.size,"%o", &filesize);

      /* and size of its blocs */
      int s = (filesize + 512 - 1)/512;
      lseek(fd,s * BLOCKSIZE,SEEK_CUR);
    }
  }
  return 1;
}
