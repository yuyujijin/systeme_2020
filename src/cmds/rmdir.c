#include "../useful.h"
#include "rmdir.h"

int main(int argc, char** argv)
{
  printf ("%d\n", has_tar(getRealPath(argv[1])));
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
      //if rmdir called without argument
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
		rmdir_tar(path, tar_index);
		
	      //if we're in a tar
	      else 
		rmdir_tar(path, tar_index);
		
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

int rmdir_tar(char *argv, int start)
{
  special_path sp = special_path_maker (argv);

  char path [strlen (sp.path) + strlen (sp.tar_name) + 1];

  path = sp.path;
  path + strlen (sp.path) = sp.tar_name;
  path [strlen (path - 1)] = '\0';
  /*write (1, argv, strlen(argv));
  //name is the name of the directory in the tar
  char name [strlen(argv) - start + 1] = substr (argv, start, strlen(argv));
  //memset (name, '\0', strlen(argv) - start + 1);
  memcpy(name, argv + start, strlen(argv) - start);
  
  //needs to end by '/'
  if(argv[strlen(argv)-1]!='/')
    name[strlen(name)]='/';
  
  //path is the path of the tarball
  char path [start + 1];
  memset (name, '\0', strlen(argv) - start + 1);
  memcpy(path,argv,strlen(argv)-(start + 1));

  write (1, "name : ", 7);
  write (1, name, strlen(name));
  write (1, "\n", 1);*/
  write (1, path, strlen(path));
  write (1, "\n", 1);
 
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
