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
	  char* path = malloc (strlen(getRealPath(argv[i])) + 2);
	  if (path == NULL)
	    {
	      perror("rmdir");
	      exit (EXIT_FAILURE);
	    }
	  strcat ( path, "/");
	  strcat ( path, getRealPath(argv[i]) );
	  strcat ( path, "\0");	  
	  
	  //if we're working in a regular path
	  if(! (has_tar(path)) && (getenv("TARNAME")[0]=='\0'))
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
		    {
		      if (is_empty_tar (path))
			execlp("rm", "rm", "-r", path, NULL);
		      else
			{
			  errno = ENOTEMPTY;
			  perror ("rmdir");
			  exit (EXIT_FAILURE);
			}
		    }
		  break;
		default :wait(NULL);break;
		}
	    }
	  //if we're in the case we have to deal with tar
	  else
	    rmdir_tar(path);
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
  special_path sp = special_path_maker (argv);

  char path[strlen(sp.path) + strlen (sp.tar_name) + 2];
  memset (path, 0, strlen(sp.path) + strlen (sp.tar_name) + 2);
  sprintf (path, "/%s%s", sp.path, sp.tar_name);

  
  if (file_exists_in_tar(path,substr(sp.tar_path,0,strlen(sp.tar_path) -1)))
    {
      errno = ENOTDIR;
      perror("rmdir");
      exit (EXIT_FAILURE);
    }

  write(1, sp.tar_path, strlen(sp.tar_path));
  if(! file_exists_in_tar(path,sp.tar_path))
    {
      errno=ENOENT;
      perror("rmdir");
      exit(EXIT_FAILURE);
    }

  //count of nb of files path/name/xxx
  // if > 1 we can't erase dir, it's not empty
  if (! is_empty (path, sp.tar_path))
    {
      errno = ENOTEMPTY;
      perror("rdmir");
      exit (EXIT_FAILURE);
    }
  
  rmTar(path, sp.tar_path);

  return 0;
}
