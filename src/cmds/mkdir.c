#include "mkdir.h"

int mkDir_call(int argc,char** argv);

int main(int argc,char** argv)
{
  return mkDir_call(argc,argv);
}

int mkDir_call(int argc,char** argv){
  for(int i = 1; i < argc; i++){
    int w;
    char *p; special_path sp;
    switch(fork()){
      case -1 : return -1;
      case 0 :
      p = getRealPath(argv[i]);
      sp = special_path_maker(p);
      if(strlen(sp.tar_path) > 0) sp.tar_path[strlen(sp.tar_path) - 1] = '\0';

      /* si on est dans un tar */
      if(strlen(sp.tar_name) > 0){
        /* le tar a ouvrir est a l'adresse "/" + pwd + nom du tar */
        char tarlocation[strlen(sp.path) + strlen(sp.tar_name) + 2];
        memset(tarlocation,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
        sprintf(tarlocation,"/%s%s",sp.path,sp.tar_name);

        return addDirTar(sp.tar_name,sp.tar_path);
      }else{
        char path[strlen(sp.path) + 2];
        memset(path,0,strlen(sp.path) + 2);
        sprintf(path,"/%s",sp.path);
        // si on l'est pas
        if(execlp("mkdir","mkdir",path,NULL)<0){
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
