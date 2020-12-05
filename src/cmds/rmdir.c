#include "rmdir.h"

int main(int argc, const char** argv)
{
  return rmdir_call(argc,argv);
}

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
		    if(execlp("rmdir","rmdir",argv[i],NULL)<0)
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
		if(rmdir_tar(argv[i],tar_index)<0)
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

int rmdir_tar(const char *argv, int tar_index)
{
  return 0;
}
