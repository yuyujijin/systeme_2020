#include "ls.h"

char *month[]={"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
int ls(char *const args[],int argc){
  int fork_ls=fork();
  switch (fork_ls) {
    case -1/* value */:perror("fork");exit(EXIT_FAILURE);
    case 0 :
      {
        int option=has_option(args,argc);
        int* tarIndex=malloc(sizeof(int)*argc);
        has_Tar(args,argc,tarIndex);
        if(argc==1){
          if(execlp("ls","ls",NULL)<0){
            perror("ls");
          }
        }
        for(int i=1;i<argc;i++){//since args={"ls",args[0]....args[argc]} so we start with i=1
          if(((argc>2&&option<0)||argc>3)&&option!=i){
            if(write(1,args[i],strlen(args[i]))<0)return -1;
            if(write(1,":\n ",2)<0)return -1;
          }
          //if we're working with a regular path
          if(tarIndex[i]==0){
            switch(fork()){
              case -1:perror("fork_loop");exit(EXIT_FAILURE);
              case 0 :
                if(option==-1&&execlp("ls","ls",args[i],NULL)<0){
                  perror(args[i]);
                }else if(i!=option&&execlp("ls","ls","-l",args[i],NULL)<0){
                  perror(args[i]);
                }
                if(i!=option&&write(1,"\n",1)<0)return -1;
                exit(EXIT_SUCCESS);
                break;
              default :wait(NULL);break;
            }
          }
          //if we're working with tar
          else if(tarIndex[i]==1){
            if(ls_tar(args[i],option)<0){
              write(1,"ls: Impossible d'accéder à '",29);
              write(1,args[i],strlen(args[i]));
              write(1,"': Aucun fichier ou dossier de ce type\n",39);
            }
            if(write(1,"\n",1)<0)return -1;
          }
        }
        exit(EXIT_SUCCESS);
        break;
      }
    default : wait(NULL);break;
  }
  return 0;
}

int has_option(char *const args[],int argc){//if option<-1 then "ls" else "ls -l"
  for(int i=0;i<argc;i++){
    if(strcmp(args[i],"-l")==0)return i;
  }
  return -1;
}

int ls_tar(const char *args,int option){
  struct posix_header** posix_header=posix_header_from_tarFile(args);
  if(posix_header==NULL)return -1;
  if(option==-1){
    for(int i=0;posix_header[i]!=NULL;i++){
      if(write(1,posix_header[i]->name,strlen(posix_header[i]->name))<0)return -1;
      if(write(1," ",1)<0)return -1;
    }
    return 0;
  }
  // now we are in the case of "ls -l"
  int max_size=maxNbDigit(posix_header);
  for(int i=0;posix_header[i]!=NULL;i++){
    unsigned long int int_time=strtol(posix_header[i]->mtime,NULL,8);
    const time_t time = (time_t)int_time;
    struct tm *date=gmtime(&time);
    unsigned int int_gid=strtol(posix_header[i]->gid,NULL,0);
    unsigned int int_uid=strtol(posix_header[i]->uid,NULL,0);
    struct group *gid=getgrgid(int_gid);//to get the gid name because info.st_gid only gives us an int
    struct passwd *uid=getpwuid(int_uid);//same
    unsigned int int_filesize=strtol(posix_header[i]->size,NULL,0);
    if(posix_header[i]->typeflag-48==5)int_filesize=4096;//Standard size for folder
    int line_size=10+2+strlen(uid->pw_name)+strlen(gid->gr_name)+max_size+3+2+3+2+strlen(posix_header[i]->name)+9;
    char line[line_size];
    char mode[11];
    convert_stmode(posix_header[i],mode);
    if(snprintf(line,sizeof(line),"%s %s %s %s %*d %s. %02d %02d:%02d %s", //we write the data from stat on the line
    mode,"1",uid->pw_name,gid->gr_name,max_size,
    int_filesize,month[date->tm_mon],date->tm_mday,date->tm_hour,date->tm_min,posix_header[i]->name)<0)return-1;
    if(write(1,line,sizeof(line))<0)return -1;
    if(write(1,"\n",1)<0)return -1;
  }
  return 0;

}
int nbDigit (int n) {//return the number of digit of a certain integer. We need it for info.st_size
    if (n == 0) return 1;
    return floor (log10 (abs (n))) + 1;
}
int maxNbDigit(struct posix_header** posix_header){// {1234,12,1} return 4
  int max=0;
  for(int i=0;posix_header[i]!=NULL;i++){
    int s=strtol(posix_header[i]->size,NULL,0);
    if(posix_header[i]->typeflag=='5')s=4096;
    if(max<nbDigit(s))max=nbDigit(s);
  }
  return max;
}
void convert_stmode(struct posix_header* posix_header,char mode[]){//0000644 -> -rx-r--r--
  if(posix_header->typeflag-48==0)mode[0]='-';
  else if(posix_header->typeflag-48==5)mode[0]='d';
  switch (posix_header->mode[4]){
    case '7':mode[1]='r';mode[2]='w';mode[3]='x';break;
    case '6':mode[1]='r';mode[2]='w';mode[3]='-';break;
    case '5':mode[1]='-';mode[2]='w';mode[3]='x';break;
    case '4':mode[1]='r';mode[2]='-';mode[3]='-';break;
    case '2':mode[1]='-';mode[2]='w';mode[3]='-';break;
    case '1':mode[1]='-';mode[2]='-';mode[3]='x';break;
  }
  switch (posix_header->mode[5]){
    case '7':mode[4]='r';mode[5]='w';mode[6]='x';break;
    case '6':mode[4]='r';mode[5]='w';mode[6]='-';break;
    case '5':mode[4]='-';mode[5]='w';mode[6]='x';break;
    case '4':mode[4]='r';mode[5]='-';mode[6]='-';break;
    case '2':mode[4]='-';mode[5]='w';mode[6]='-';break;
    case '1':mode[4]='-';mode[5]='-';mode[6]='x';break;
  }
  switch (posix_header->mode[6]){
    case '7':mode[7]='r';mode[8]='w';mode[9]='x';break;
    case '6':mode[7]='r';mode[8]='w';mode[9]='-';break;
    case '5':mode[7]='-';mode[8]='w';mode[9]='x';break;
    case '4':mode[7]='r';mode[8]='-';mode[9]='-';break;
    case '2':mode[7]='-';mode[8]='w';mode[9]='-';break;
    case '1':mode[7]='-';mode[8]='-';mode[9]='x';break;
  }
  mode[10]='\0';
}
int main(int argc,char *const argv[]){
  return ls(argv,argc);
}
