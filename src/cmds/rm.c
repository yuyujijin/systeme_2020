#include"rm.h"

int is_empty(const char *path);

int main(int argc, const char** argv)
{
  return rm_call(argc,argv);
}

int option_r (const char * argv)
{
  if (argv[0] == '-')
    {
      if (argv[1] == 'r')
	return 1;
      else
	return -1;
    }
  else return 0;
}

//call rm if we're in a regular path
//else calls rm
int rm_call(int argc,const char** argv)
{
  int fork_rmdir=fork();
  switch(fork_rmdir)
    {
    case -1: //error
      perror("rm");
      exit(EXIT_FAILURE);
    case 0: //son
      {
	int option = option_r (argv[1]);
	if (option == -2)
	  {
	    perror("rm : wrong option");
	    exit (EXIT_FAILURE);
	  }
	//if rm called without argument
	if(argc==1)
	  {
	    errno=22; //invalid argument
	    perror("rm");
	    exit(EXIT_FAILURE);
	  }
	//for each arg of argv
	int i = 1;
	if (option == 1)
	  i = 2;
	for (;i<argc;i++)
	  {	    
	    //if we're working in a regular path
	    int tar_index = has_tar(argv[i]);
	    //if we want delete a tar with rm -> error
	    if (last_is_tar(argv[i]) && option == 0)
	      {
		errno=EISDIR;
		perror("rm");
		exit(EXIT_FAILURE);
	      }
	    else if(!tar_index && (getenv("TARNAME")[0]=='\0') )
	      {
		switch(fork())
		  {
		  case -1:
		    perror("rm");
		    exit(EXIT_FAILURE);
		  case 0 :
		    if(execlp("rm","rm",argv[i],NULL) < 0)
		      {
			perror("rm");
			exit(EXIT_FAILURE);
		      }
		    break;
		  default :wait(NULL);break;
		  }
	      }
	    //if we're in the case we have to deal with tar
	    else
	      {
		write(1,getRealPath(argv[i]),strlen(getRealPath(argv[i])));
		write(1,"\n",1);
		//we create pathname, under the form
		// "tarname/tarpath/argv[i]"
		char *pathname = malloc(strlen (getenv ("TARPATH"))
					+ strlen (getenv("TARNAME"))
					+ strlen (argv[i])
					+ 2);
		if (pathname == NULL)
		  {
		    perror ("rm");
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
	       
		//if we're not in a tar
		if (getenv("TARNAME")[0]=='\0') {
		  switch (option)
		    {
		    case 0 :
		      rm_tar(argv[i],tar_index); break;
		    case 1 :
		      rm_tar_option(argv[i],tar_index); break;
		    default : break;
		    }
		}
		//if we're in a tar
		else {
		   switch (option)
		    {
		    case 0 :
		      rm_tar(pathname,strlen(getenv("TARNAME"))+1); break;
		    case 1 :
		      rm_tar_option(pathname,strlen(getenv("TARNAME"))+1); break;
		    default : break;
		    }
		}
		free (pathname);
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
      struct stat sb;

      if (lstat(argv, &sb) == -1) {
	perror("rm : lstat");
	exit(EXIT_FAILURE);
      }

      if (S_ISREG(sb.st_mode)) {
        return 1;
      }

    }
  return 0;
}



int rm_tar(const char *argv, int start)
{
  //name is the name of the file in the tar
  char* name=malloc(strlen(argv)-start+1);
  memset(name,'\0',strlen(argv)-start+1);
  memcpy(name,argv+start,strlen(argv)-start+1);
  
  //path is the path of the tarball
  char *path=malloc(strlen(argv)-start+1);
  memcpy(path,argv,strlen(argv)-(strlen(argv)-start+1));

  //check if the path exists in the tarball
  if(! file_exists_in_tar(path,name))
    {
      //we first suppose it's a dir
      //needs to end by '/'
      if(argv[strlen(argv)-1]!='/')
	name[strlen(name)]='/';
      if(! file_exists_in_tar(path,name))
	{
	  free(name);
	  free(path);
	  errno=ENOENT;
	  perror("rm");
	  exit(EXIT_FAILURE);
	}
      else
	{
	  free(name);
	  free(path);
	  errno = EISDIR;
	  perror ("rm : impossible de supprimer");
	  exit (EXIT_FAILURE);
	}
    }

  struct posix_header hd;
  int fd;

  fd = open(path,O_RDONLY);
  //if tarball path doesn't exist
  if(fd<0)
    {
      free(name);
      free(path);
      close(fd);
      errno=17;
      perror("rm");
      exit(EXIT_FAILURE);
    }
  
  while(read(fd, &hd, sizeof(struct posix_header))){
  
    if(hd.name[0]=='\0')
      {
	break;
      }

    if (strlen (hd.name) == strlen (name))
      {
	for (unsigned int i = 0; i < strlen (name) ; i ++)
	  {
	    if (hd.name[i] != name[i]) break;
	  }
	
	rmTar(path, name);
	
	free(name);
	free(path);
	close(fd);
	return 0;
      }

    int filesize;
    sscanf(hd.size,"%d", &filesize);
    int s = (filesize + 512 - 1)/512;
    struct posix_header* temp = malloc(sizeof(struct posix_header) * s);
    read(fd, temp, s * BLOCKSIZE);
    free(temp);
  } 
  close (fd);
  free(name);
  free(path);
  errno = 17;
  perror ("rm");
  exit (EXIT_FAILURE);
    
}

int rm_tar_option(const char *argv, int start)
{
  //name is the name of the file in the tar
  char* name=malloc(strlen(argv)-start+1);
  memset(name,'\0',strlen(argv)-start+1);
  memcpy(name,argv+start,strlen(argv)-start+1);
  
  //path is the path of the tarball
  char *path=malloc(strlen(argv)-start+1);
  memcpy(path,argv,strlen(argv)-(strlen(argv)-start+1));
  
  int isdir = 0;
  //check if the path exists in the tarball
  if(! file_exists_in_tar(path,name))
    {
      //we first suppose it's a dir
      //needs to end by '/'
      if(argv[strlen(argv)-1]!='/')
	name[strlen(name)]='/';

      isdir = 1;

      if(! file_exists_in_tar(path,name))
	{
	  errno=ENOENT;
	  perror("rm");
	  exit(EXIT_FAILURE);
	}
    }

  if (! isdir)
    {
      free(name);
      free(path);
      return 0;
      return rm_tar (argv, start);
    }
  
  struct posix_header hd;
  int fd;

  fd = open(path,O_RDONLY);
  //if tarball path doesn't exist
  if(fd < 0)
    {
      close (fd);
      free(name);
      free(path);
      errno=17;
      perror("rmdir");
      exit(EXIT_FAILURE);
    }

  //we put in rm_adress all the & we have to delete
  char* rm_adress[512];

  int index = 0;
  
  while(read(fd, &hd, sizeof(struct posix_header))){	
    
    for (unsigned i = 0; i < strlen (name); )
      {
	if (hd.name[i] == name[i] && i == (strlen (name) - 1))
	  {
	    rm_adress[index ++] = strdup(hd.name);
	  }
	else if (hd.name[i] != name[i])
	  {
	    i = strlen(name);
	    int filesize;
	    sscanf(hd.size,"%d", &filesize);
	    int s = (filesize + 512 - 1)/512;
	    struct posix_header* temp = malloc(sizeof(struct posix_header) * s);
	    read(fd, temp, s * BLOCKSIZE);
	    free(temp); 
	  }
	i++;	 
      }
  }


  if (index <= 2)
    {
      rmTar(path, name);
      if (is_empty(path))
	execlp("rm","rm",path,NULL);
    }
  else
    {
      for(int i = 0; i < index; i++)
	{
	  char argv0[strlen (path) + 1 + strlen(rm_adress[i]) + 1];
	  memset(argv0,0,+ strlen(rm_adress[i]) + 1);
	  sprintf(argv0,path,"%s/%s",rm_adress[i]);

	  int w;
	  // et on rapelle recursivement sur chaque fichier
	  //printf("%s %s\n",newargv[0],newargv[1]);
	  int r = fork();
	  switch(r){
	  case -1 : return -1;
	  case 0 :rmTar(path, rm_adress[i]);break;
	  default : waitpid(r,&w, 0); if(WEXITSTATUS(w) == 255) return -1; break;
	  }
	}
    }

  close (fd);

  if (is_empty(path))
    execlp("rm","rm",path,NULL);
  free(name);
  free(path);
  return 0;
    
}

int is_empty(const char *path)
{
  int fd = open(path,O_RDONLY);
  //if tarball path doesn't exist
  if(fd < 0)
    {
      close (fd);
      errno=17;
      perror("rmdir");
      exit(EXIT_FAILURE);
    }

   struct posix_header hd;
   int ret = read(fd, &hd, sizeof(struct posix_header));
   close(fd);
   return (ret);

  
}
