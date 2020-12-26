#include "mkdir.h"

int mkDir_call(int argc,char** argv);

int main(int argc,char** argv)
{
  return mkDir_call(argc,argv);
}

int mkDir_call(int argc,char** argv){
  for(int i = 1; i < argc; i++){
    char *last_arg;
    int w;
    switch(fork()){
      case -1 : return -1;
      case 0 :
      last_arg = getLastArg(argv[i]);
      // si le dernier argument != argv[i] (on tente d'y acceder)
      if(strcmp(argv[i],last_arg)) if(cd(pathminus(argv[i],last_arg)) < 0) exit(EXIT_FAILURE);
      // si on est dans un tar
      if(strlen(getenv("TARNAME"))){
        char dirname[strlen(getenv("TARPATH")) + strlen(last_arg) + 2];
        memset(dirname,0,strlen(getenv("TARPATH")) + strlen(last_arg) + 2);

        strcat(dirname,getenv("TARPATH"));
        strcat(dirname,last_arg);
        strcat(dirname,"/");

        return addDirTar(getenv("TARNAME"),dirname);
      }else{
        // si on l'est pas
        if(execlp("mkdir","mkdir",last_arg,NULL)<0){
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
      }
      exit(-1);
      default : wait(&w); break;
    }
  }
  return 1;
}

int addDirTar(char* path, char* name)
{
  if(exists(path,name) > 0) return -1;

  int fd;

  fd = open(path,O_WRONLY | O_RDONLY);
  if(fd < 0) return 0;

  //we get the offset right before the empty blocks
  size_t offt = offsetTar(path);

  // and we go there
  lseek(fd,offt,SEEK_SET);

  struct posix_header hd;
  memset(&hd,'\0',sizeof(struct posix_header));

  memcpy(hd.name, name, strlen(name));
  sprintf(hd.mode,"0000700");

  sprintf(hd.size, "%011o", 0);

  hd.typeflag = '5';
  memcpy(hd.magic,"ustar",5);
  memcpy(hd.version,"00",2);
  set_checksum(&hd);

  if(check_checksum(&hd) < 0) return -1;

  //put the header in the tarball
  if (write(fd,&hd,sizeof(struct posix_header))<0)
    perror("mkdir");

  char emptybuf[BLOCKSIZE];
  memset(emptybuf,0,BLOCKSIZE);
  for(int i = 0; i < 2; i++){ if(write(fd, emptybuf,BLOCKSIZE) < 0) return -1; }

  close(fd);
  return 1;

  return 0;
}
