#include "mkdir.h"
#include"tar_manipulation.c"

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

int has_tar(const char* argv)
{
  //loop until |argv|-5 if we want creat a
  //directory "xxx.tar"
  for(int i=0;i<strlen(argv)-5;i++)
    {
      if(argv[i]=='.' && argv[i+1]=='t' && argv[i+2]=='a' && argv[i+3]=='r'){
	//check if the path is a directory name "xxx.tar" or just
	//a tar
	int fd=open(substr(argv,0,i+5),O_DIRECTORY);
	if (fd<0){
	  close(fd);
	  return i+5;
	}
      }
    }
  return 0;
}

int mkdir_tar(const char* argv,int start)
{
  //if directory already exists in the tar
  
  char* name=malloc(strlen(argv)-start+1);
  memset(name,'\0',strlen(argv)-start+1);
  memcpy(name,argv+start,strlen(argv)-start+1);
  
  if(argv[strlen(argv)-1]!='/')
    name[strlen(name)]='/';

  char *path=malloc(strlen(argv)-start+1);
  memcpy(path,argv,strlen(argv)-(strlen(argv)-start+1));
  //printf("name: %s\npath: %s\n",name,path);
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

  size_t size = read(fd, &hd, sizeof(struct posix_header));

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

char *substr(const char *src,int start,int end) { 
  char *dest=NULL;
  if (end-start>0) {                   
    dest = malloc(end-start+1);
    if(dest==NULL) perror("mkdir");
    for(int i=0;i<end-start;i++){
      dest[i]=src[start+i];
    }
    dest[end]='\0';
  }
  return dest;                            
}

int file_exists_in_tar(char* path, char* name){
  struct posix_header hd;
  int fd;

  fd = open(path,O_RDONLY);

  while(hd.name[0]!='\0'){
    
    size_t size = read(fd, &hd, sizeof(struct posix_header));

    if(strcmp(name,hd.name)==0)  { close(fd); return 1;}

    int filesize;
    sscanf(hd.size,"%o", &filesize);
    int s = (filesize + 512 - 1)/512;
    struct posix_header* temp = malloc(sizeof(struct posix_header) * s);
    read(fd, temp, s * BLOCKSIZE);
    free(temp);
  }
  return 0;
}

/*
int nextSpace(const char* path,int index)
{
  for(int i=index;i<strlen(path)+1;i++)
    {
      if(path[i]==' '||path[i]=='\0'){
	return i;
      }
    }
  return index;
}

int mkDirectory(const char* argv)
{
  char path[PATH_MAX+1];//directory path
  snprintf(path, PATH_MAX + 1,"%s",argv);

  struct stat stDir;
 
  if(stat(path, &stDir)==-1) //if name doesn't exist
    {
      if(mkdir(path, 0755) == -1) perror("mkdir");
      return 1;
    }
  else
    {
      errno=17;
      perror("mkdir");
      return -1;
    }
}

int splitMkDir(const char* argv)
{
  int start=0;
  int end;
  while(start<strlen(argv))
    {
      end=nextSpace(argv, start);
      mkDirectory(substr(argv,start,end));
      start=end+1;
    }
}
*/
int main(int argc, const char** argv)
{
  //erreur si zero argument à voir au moment du lien
  mkDir_call(argc,argv);

  //perror(cmd);
  
  //printf("%s\n",cmd);
  return 0;
}
