#include "mkDir.h"
#include "tar_manipulation.h"
//#include "tar.h"

int mkDir_call(char *const arg[],int argc){
  int fork_mkDir=fork();
  switch(fork_mkDir)
    {
    case -1:perror("mkdir");
      exit(EXIT_FAILURE);
    case 0:
      {
	int option=has_option(args, argc);
	int* tarIndex=malloc(sizeof(int)*argc);
	if(tarIndex==NULL)
	  {
	    perror("mkDir");
	    exit(EXIT_FAILURE);
	  }
	has_Tar(args,argc,tarIndex);
	if(argc==1)
	  {
	    perror("mkDir OPERANDE MANQUANTE");
	    exit(EXIT_FAILURE);
	  }
	else if(argc>1 && option==0)
	  {
	    if(exevp("mkdir",
	  }
	    
      }
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
  splitMkDir(argv[1]);
  return 0;
}
