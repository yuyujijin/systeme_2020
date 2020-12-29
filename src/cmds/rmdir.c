#include "../useful.h"
#include "rmdir.h"

int main(int argc, char** argv)
{
  return rmdir_call(argc,argv);
}

//call rmdir id we're in a regular path
//else calls rmdir path
int rmdir_call(int argc,char** argv)
{
  int fork_rmdir=fork();
  switch(fork_rmdir)
    {
    case -1: //error
      perror("rmdir");
      exit(EXIT_FAILURE);
    case 0: //son
      //if rmdir is called without argument
      if(argc==1)
	{
	  errno=22; //invalid argument
	  perror("rmdir");
	  exit(EXIT_FAILURE);
	}

      //for each arg of argc
      for (int i=1;i<argc;i++)
	{
	  char* path = malloc (strlen(getRealPath(argv[i])) + 1);
	  if (path == NULL)
	    {
	      perror("rmdir");
	      exit (EXIT_FAILURE);
	    }

	  strcat ( path, getRealPath(argv[i]) );
	  strcat ( path, "\0");	  
	  
	  //if we're working in a regular path
	  int tar_index = has_tar(path);
	  if(!tar_index && (getenv("TARNAME")[0]=='\0'))
	    {
	      switch(fork())
		{
		case -1:
		  perror("rmdir");
		  exit(EXIT_FAILURE);
		case 0 :
		  //if the path point a tar, delete the tar
		  // it can be a tar or a dir called xx.t
		  if(last_is_tar(path))
		    execlp("rm", "rm", "-r", argv[i], NULL);
		  break;
		default :wait(NULL);break;
		}
	    }
	  //if we're in the case we have to deal with tar
	  else
	    {
	      //if we're not in a tar
	      if (getenv("TARNAME")[0]=='\0') 
		rmdir_tar(path);
		
	      //if we're in a tar
	      else 
		rmdir_tar(path);
		
	    }
	  free (path);
	}

      exit(EXIT_SUCCESS);	
    default: wait(NULL);break;
    }
return 0;
}

int last_is_tar(char* argv)
{
  if (argv[strlen(argv) - 4] == '.'
      && argv[strlen(argv) - 3] == 't'
      && argv[strlen(argv) - 2] == 'a'
      && argv[strlen(argv) - 1] == 'r')
    {
      return 1;
    }
  else return 0;
}

int rmdir_tar(char *argv)
{
  write (1, argv, strlen (argv));
  write (1, "\n", 1);
  special_path sp = special_path_maker (argv);

  char path[strlen(sp.path) + 2];
  memset(path,0,strlen(sp.path) + 2);
  sprintf(path,"/%s",sp.path);
  path[strlen(path) - 1] = '\0';

  write (1, path, argv (path));
  write (1, "\n", 1);
  if(! file_exists_in_tar(path,sp.tar_path))
    {
      errno=ENOENT;
      perror("rmdir");
      exit(EXIT_FAILURE);
    }


  //count of nb of files path/name/xxx
  // if > 1 we can't erase dir, it's not empty
  if (! is_empty (path, sp.tar_name))
    {
      errno = ENOTEMPTY;
      perror("rdmir");
      exit (EXIT_FAILURE);
    }
  
  rmTar(path, sp.tar_path);

  return 0;
}
