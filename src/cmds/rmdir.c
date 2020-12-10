#include "rmdir.h"

int main(int argc, const char** argv)
{
  return rmdir_call(argc,argv);
}

//call rmdir id we're in a regular path
//else calls rmdir path
int rmdir_call(int argc,const char** argv)
{
  int fork_rmdir=fork();
  switch(fork_rmdir)
    {
    case -1: //error
      perror("rmdir");
      exit(EXIT_FAILURE);
    case 0: //son
      {
	//if rmdir called without argument
	if(argc==1)
	  {
	    errno=22; //invalid argument
	    perror("rmdir");
	    exit(EXIT_FAILURE);
	  }


	char* cmd="rmdir ";
	//cmd="rmdir ";
	
	char* arg=malloc(PATH_MAX);
	if(arg==NULL || cmd==NULL)
	  {
	    perror("rmdir");
	    exit(EXIT_FAILURE);
	  }

	//copy of cmd in arg
	//arg is the final entire command line
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
		    perror("rmdir");
		    exit(EXIT_FAILURE);
		  case 0 :
		    //if the path point a tar, delete the tar
		    printf ("%d\n", last_is_tar(argv[i]));
		    if(! last_is_tar(argv[i]) &&
		       execlp("rmdir","rmdir",argv[i],NULL) < 0)
		      {
			perror("rmdir");
			exit(EXIT_FAILURE);
		      }
		    break;
		  default :wait(NULL);break;
		  }
	      }
	    //if we're in a tar
	    else
	      {
		if(rmdir_tar(argv[i],tar_index) < 0)
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

int last_is_tar(const char* argv)
{
  printf("%s\n",argv);
  if (argv[strlen(argv) - 4] == '.'
      && argv[strlen(argv) - 3] == 't'
      && argv[strlen(argv) - 2] == 'a'
      && argv[strlen(argv) - 1] == 'r')
    {
      if (execlp("rm","rm",argv,NULL)<0)
	{
	  perror("rmdir");
	  exit(EXIT_FAILURE);
	}
      return 1;
    }
  else return 0;
}

int rmdir_tar(const char *argv, int start)
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
  printf("%s\n%s\n", path, name);
  if(! file_exists_in_tar(path,name))
    {
      errno=ENOENT;
      perror("rmdir");
      exit(EXIT_FAILURE);
    }

  struct posix_header hd;
  int fd;

  fd = open(path,O_RDONLY);
  //if tarball path doesn't exist
  if(fd<0)
    {
      close(fd);
      errno=17;
      perror("rmdir");
      exit(EXIT_FAILURE);
    }
  
  while(read(fd, &hd, sizeof(struct posix_header))){
    if(hd.name[0]=='\0')
      return 0;

    for (unsigned int i = 0; i < strlen (hd.name) ; i ++)
      {
	if (hd.name[i] != name[i]) break;
	else if (i == strlen (hd.name) - 1)
	  rmTar(path, name);
      }

    int filesize;
    sscanf(hd.size,"%d", &filesize);
    int s = (filesize + 512 - 1)/512;
    struct posix_header* temp = malloc(sizeof(struct posix_header) * s);
    read(fd, temp, s * BLOCKSIZE);
    free(temp);
    close (fd);
  }
  return 0;

  free(name);
  return 0;
}
