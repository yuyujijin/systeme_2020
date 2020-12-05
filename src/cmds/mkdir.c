#include "mkdir.h"

int mkDir_call(int argc,const char** argv);

int main(int argc, const char** argv)
{
  return mkDir_call(argc,argv);
}

int mkDir_call(int argc,const char** argv){
  //fork for exec "mkdir
  int fork_mkDir=fork();
  switch(fork_mkDir)
    {
    case -1: //error
      perror("mkdir");
      exit(EXIT_FAILURE);
    case 0: //son
      {
	//if mkdir called without argument
	if(argc==1)
	  {
	    errno=22; //invalid argument
	    perror("mkdir");
	    exit(EXIT_FAILURE);
	  }
	char* cmd=malloc(6);
	cmd="mkdir ";
	char* arg=malloc(PATH_MAX);
	if(arg==NULL || cmd==NULL) exit(EXIT_FAILURE);
	memcpy(arg,cmd,6);
	for (int i=1;i<argc;i++)
	  {
	    //if we're working in a regular path
	    int tar_index=has_tar(argv[i]);
	    if(!tar_index)
	      {
		switch(fork())
		  {
		  case -1:
		    perror("mkdir");
		    exit(EXIT_FAILURE);
		  case 0 :
		    if(execlp("mkdir","mkdir",argv[i],NULL)<0)
		      {
			perror("mkdir");
			exit(EXIT_FAILURE);
		      }
		    break;
		  default :wait(NULL);break;
		  }
	      }
	    //if we're in a tar
	    else
	      {
		if(mkdir_tar(argv[i],tar_index)<0)
		  {
		    perror("A REFLECHIIIRRR");
		  }
	      }
	  }
	exit(EXIT_SUCCESS);
	break;
      }
    default: wait(NULL);break;
    }
  return 0;
}



int mkdir_tar(const char* argv,int start)
{
  //name is the name of the directory in the tar
  char* name=malloc(strlen(argv)-start+1);
  memset(name,'\0',strlen(argv)-start+1);
  memcpy(name,argv+start,strlen(argv)-start+1);

  //needs to end by '/'
  if(argv[strlen(argv)-1]!='/')
    name[strlen(name)]='/';

  //path is the path of the tarball
  char *path=malloc(strlen(argv)-start+1);
  memcpy(path,argv,strlen(argv)-(strlen(argv)-start+1));

  //check if the path exists in the tarball
  if(file_exists_in_tar(path,name))
    {
      errno=17;
      perror("mkdir");
      exit(EXIT_FAILURE);
    }

  if(!addDirTar(path,name)){
    perror("mkdir");
    exit(EXIT_FAILURE);
  }
  return 0;
}

int addDirTar(char* path, char* name)
{
  int fd;

  fd = open(path,O_WRONLY | O_RDONLY);
  if(fd < 0) return 0;

  //we get the offset right before the empty blocks
  size_t offt = offsetTar(path) - BLOCKSIZE*2;
  // and we go there
  lseek(fd,offt + BLOCKSIZE,SEEK_CUR);

  //create new header for the new directory
  struct posix_header hd;
  memset(&hd,'\0',sizeof (struct posix_header));

  read(fd, &hd, sizeof(struct posix_header));

  memset(&(hd.name),'\0',100);
  memcpy(hd.name, name, strlen(name)+1);

  sprintf(hd.mode,"0000700");

  sprintf(hd.size,"%011o",4096); //taille a voir
  //memcpy(hd.size,"%011o");

  hd.typeflag ='5';
  memcpy(hd.magic,TMAGIC,6);
  set_checksum(&hd);

  //put the header in the tarball
  if (write(fd,&hd,sizeof(struct posix_header))<0)
    perror("mkdir");

  char emptybuf[512];
  memset(emptybuf,0,512);
  for(int i = 0; i < 2; i++){ if(write(fd, emptybuf,512) < 0) return -1; }

  close(fd);
  return 1;

  return 0;
}


