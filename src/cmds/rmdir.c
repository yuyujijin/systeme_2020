
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
	/*
	//else
	char* cmd="rmdir ";
	
	char* arg=malloc(PATH_MAX);
	if(arg==NULL)
	  {
	    perror("rmdir");
	    exit(EXIT_FAILURE);
	  }

	//copy of cmd in arg
	//arg is the final entire command line
	memcpy(arg,cmd,6);
	*/
	//for each arg of argc
	for (int i=1;i<argc;i++)
	  {
	    char *pathname = malloc(strlen (getenv ("TARPATH"))
				    + strlen (getenv("TARNAME"))
				    + strlen (argv[i])
				    + 2);
	    if (pathname == NULL)
	      {
		perror ("rmdir");
		exit (EXIT_FAILURE);
	      }

	    memset(pathname, '\0', sizeof(strlen (getenv ("TARPATH"))
					  + strlen (getenv("TARNAME"))
					  + strlen (argv[i])
					  + 2));
	    strcat (pathname, getenv("TARNAME"));
	    strcat (pathname, "/");
	    strcat (pathname, getenv("TARPATH"));
	    strcat (pathname, argv[i]);
	    
	    //if we're working in a regular path
	    int tar_index = has_tar(argv[i]);
	    if(!tar_index && (getenv("TARNAME")[0]=='\0'))
	      {
		switch(fork())
		  {
		  case -1:
		    free(pathname);
		    perror("rmdir");
		    exit(EXIT_FAILURE);
		  case 0 :
		    //if the path point a tar, delete the tar
		    if(! last_is_tar(argv[i]) &&
		       execlp("rmdir","rmdir",argv[i],NULL) < 0)
		      {
			free(pathname);
			perror("rmdir");
			exit(EXIT_FAILURE);
		      }
		    free(pathname);
		    break;
		  default :wait(NULL);break;
		  }
	      }
	    //if we're in the case we have to deal with tar
	    else
	      {
		//if we're not in a tar
		if (getenv("TARNAME")[0]=='\0') {
		  if(rmdir_tar(argv[i],tar_index) < 0)
		    {
		      perror("A REFLECHIIIRRR");
		      }
		}
		//if we're in a tar
		else {
		  if(rmdir_tar(pathname,strlen(getenv("TARNAME"))+1) < 0)
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

  //count of nb of files path/name/xxx
  // if > 1 we can't erase dir, it's not empty
  unsigned int count = 0;
  while(read(fd, &hd, sizeof(struct posix_header))){
  
    if(hd.name[0]=='\0')
      {
	break;
      }

    //verify that dir name is empty
    //that doesn't exist file name/xxx
    if (strlen (hd.name) >= strlen (name))
      {
	for (unsigned int i = 0; i < strlen (name) ; i ++)
	  {
	    if (hd.name[i] != name[i]) break;
	    else if (i == strlen (name) - 1)
	      count ++;
	  }
	if (count > 1)
	  {
	      errno=ENOTEMPTY;
	      perror ("rmdir");
	      free(name);
	      free(path);
	      close(fd);
	      exit(EXIT_FAILURE);
	    }
      }

    int filesize;
    sscanf(hd.size,"%d", &filesize);
    int s = (filesize + 512 - 1)/512;
    struct posix_header* temp = malloc(sizeof(struct posix_header) * s);
    read(fd, temp, s * BLOCKSIZE);
    free(temp);
  }

  rmTar(path, name);
  close (fd);
  free(name);
  free(path);
  return 0;
}
