#include "mkdir.h"
#include "tar_manipulation.h"
//#include "tar.h"

int mkDir_call(int argc,const char** argv){
  int fork_mkDir=fork();
  switch(fork_mkDir)
    {
    case -1:perror("mkdir");
      exit(EXIT_FAILURE);
    case 0:
      {
	if(argc==1)
	  {
	    perror("mkDir OPERANDE MANQUANTE");
	    exit(EXIT_FAILURE);
	  }
	else if(argc>1)
	  {
	    char* cmd=malloc(6);
	    cmd="mkdir ";
	    char* arg=malloc(PATH_MAX);
	    if(arg==NULL || cmd==NULL) exit(EXIT_FAILURE);
	    memcpy(arg,cmd,6);
	    for (int i=1;i<argc;i++)
	      {
		//if we're working in a regular path
		if(!(has_tar(argv[i]))) 
		  {
		    switch(fork())
		      {
		      case -1:perror("mkdir");exit(EXIT_FAILURE);
		      case 0 :
			if(execlp("mkdir","mkdir",argv[i],NULL)<0)
			  {
			    perror("mkdir");
			  }
			break;
		      default :wait(NULL);break;
		      }
		  }
		else
		  {
		    if(mkdir_tar(argv[i])<0)
		      {
			perror("A REFLECHIIIRRR");
		      }
		  }
	      }
	    exit(EXIT_SUCCESS);
	    break;
	  }
	else
	  {
	    errno=2;
	    perror("mkdirA VERIFIIIIIIIIIIIIIIIIIIIIIIIIIER");
	  }
      }
    default: wait(NULL);break;
    }
  return 0;
}

int has_tar(const char* argv)
{
  for(int i=0;i<strlen(argv)-3;i++)
    {
    if(argv[i]=='.' && argv[i+1]=='t' && argv[i+2]=='a' && argv[i+3]=='r')
      return 1;
    }
  
  return 0;
}

int mkdir_tar(const char* argv)
{
  int start=0;
  for(int i=0;i<strlen(argv);i++)
    {
      if(argv[i]=='/'&&i!=strlen(argv)-1)
	start=i+1;
    }
  char* name=malloc(strlen(argv)-start+1);
  memcpy(name,argv+start,strlen(argv)-start+1);

  char *path=malloc(strlen(argv)-start+1);
  memcpy(path,argv,strlen(argv)-start+1);
  addTar(path,name,'5');
  return 0;
  /*int fd;
  
  //initialize checksum
  set_checksum(&hd);
   //create struc for the new directory
  struct posix_header hd;
  memset(&hd,'\0',sizeof(struct posix_header));

  memcpy(hd.name, name, strlen(name));

  sprintf(hd.mode,"0000700");

  sprintf(hd.size,"4096");

  hd.typeflag ='5';
  memcpy(hd.magic,"ustar",5);
  memcpy(hd.version,"00",2);
  set_checksum(&hd);
  
  int fd=stat(argv,h);
  
  return 0;*/
}

/*
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
