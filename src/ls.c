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
      if(option==0){
        if(write(1,entry->d_name,strlen(entry->d_name))<0)return -1;//if we called "ls" then we display all the file name in a single row
        if(write(1," ",1)<0)return -1;
      }else{
        //else we display it like the commande ls -l does
        // if(chdir(path)<0){
        //   perror(path);
        // }
        struct stat info;
        char stat_path[strlen(path)+strlen(entry->d_name)];
        snprintf(stat_path,strlen(path)+strlen(entry->d_name)+2,"%s/%s",path,entry->d_name);
        if(stat(stat_path,&info)<0){
          perror(stat_path);
          return -1;
        }
        struct group *gid=getgrgid(info.st_gid);//to get the gid name because info.st_gid only gives us an int
        struct passwd *uid=getpwuid(info.st_uid);//same
        struct tm *date=gmtime(&info.st_mtime);//to get the date displaied as such: mmm ddd hh:minmin from st_ctime because st_ctime is a long int
        int size=7+2+strlen(uid->pw_name)+strlen(gid->gr_name)+nbDigit(info.st_size)+3+2+2+strlen(entry->d_name);
        char line[size+10];//we store the line to be written and add +10 for every space etc
        if(snprintf(line,sizeof(line),"%08o %ld %s %s %ld %s. %02d:%02d %s", //we write the data from stat on the line
          info.st_mode,info.st_nlink,uid->pw_name,gid->gr_name,
          info.st_size,month[date->tm_mon],date->tm_hour,date->tm_min,entry->d_name)<0)return-1;
        if(write(1,line,size+10)<0)return -1;
        if(write(1,"\n",1)<0)return -1;
      }
    }
  }
  if(write(1,"\n",1)<0)return -1;
  free(entry);
  if(closedir(dir)<0){
    perror(path);
    return -1;
  }
  return 0;
}
int nbDigit (int n) {//return the number of digit of a certain integer. We need it for info.st_size
    if (n == 0) return 1;
    return floor (log10 (abs (n))) + 1;
}
void ls_tar(char *path,int option){//we use this function to deal with tar file
  /*
    TO DO :
      use the struct header of a tar file done in tp1 with low-level programming
  */
}
