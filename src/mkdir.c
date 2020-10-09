#include <stdlib.h>
#include <stdio.h>
#include<limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int mkDirectory(const char* argv)
{
  char path[PATH_MAX+1];//directory path
  snprintf(path, PATH_MAX + 1,"%s",argv);

  struct stat stDir;
  if(stat(path, &stDir) !=-1)//if the directory exists
    {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }

    if(mkdir(path, 0755) == -1)
      perror("mkdir");
  
  return 0;

}

int main(int argc, const char** argv)
{
  mkDirectory(argv[1]);
  return 0;
}
