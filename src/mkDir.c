
#include "tar_manipulation.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
//#include "tar.h"

int has_tar(const char* argv)
{
  for(int i=0;i<strlen(argv)-3;i++)
    {
    if(argv[i]=='.' && argv[i+1]=='t' && argv[i+2]=='a' && argv[i+3]=='r')
      return 1;
    }
  
  return 0;
}

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
		if(!(has_tar(cmd))) 
		  {
		    if(execl("/bin/mkdir","/bin/mkdir",argv[i],NULL)==-1)
		      {
			perror("mkDir");
			exit(EXIT_FAILURE);
		      }
		  }
		else
		  {
		    printf("We're in a tar path\n");
		  }
		//free(cmd[1]);
		/*
		//si pas tar
		if(execl("mkdir",argv[i],NULL)==-1)
		  {
		    perror("mkDir");
		    exit(EXIT_FAILURE);
		  }
	    //else
	    //pour i allant jusqu'au nombre d'arg
	    //mkDir_tar(const char *argv);*/
	    }
	  }
	exit(EXIT_SUCCESS);
	break;
      }
    default: wait(NULL);break;
    }
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

int main(int argc, const char** argv)
{
  //erreur si zero argument à voir au moment du lien
  mkDir_call(argc,argv);

  //perror(cmd);
  
  //printf("%s\n",cmd);
  return 0;
}
