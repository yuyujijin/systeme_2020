#include "ls.h"
char *month[]={"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

int ls(char *path,int option){//option=1 --> ls -l     option=0 --> ls
  DIR *dir=opendir(path);//open current directory
  if(dir==NULL){
    perror(path);
    return -1;
  }

  struct dirent *entry;
  while((entry=readdir(dir))!=NULL){//for every entry in the directory
    if(strcmp(entry->d_name,".")!=0&&strcmp(entry->d_name,"..")!=0){// We don't deal with directories '.' and '..'
      if(option==0)printf(" %s ",entry->d_name);//if we called "ls" then we display all the file name in a single row
      else{
        //else we display it like the commande ls -l does
        struct stat info;
        if(stat(entry->d_name,&info)<0){
          perror(entry->d_name);
          return -1;
        }
        struct group *gid=getgrgid(info.st_gid);//to get the gid name because info.st_gid only gives us an int
        struct passwd *uid=getpwuid(info.st_uid);//same
        struct tm *date=gmtime(&info.st_ctime);//to get the date displaied as such: mmm ddd hh:minmin from st_ctime because st_ctime is a long int
        printf("%o %ld %s %s %ld %s. %d:%d %s",info.st_mode,info.st_nlink,uid->pw_name,gid->gr_name,info.st_size,month[date->tm_mon],date->tm_hour,date->tm_min,entry->d_name);
        printf("\n");
      }
    }
  }
  printf("\n");
  free(entry);
  closedir(dir);
  return 0;
}
