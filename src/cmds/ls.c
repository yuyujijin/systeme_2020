#include "ls.h"

char *month[]={"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

int ls(int argc, char **argv){
  int L = optionL(argc,argv);
  if(argc == 1 || (argc == 2 && !strcmp(argv[1],"-l"))){
    if(strlen(getenv("TARNAME")) > 0){
      ls_tar(getenv("TARNAME"),getenv("TARPATH"),L);
    }else{
      if(!L) execlp("ls","ls",NULL);
      execlp("ls","ls","-l",NULL);
      exit(-1);
    }
  }
  for(int i = 1; i < argc; i++){

    if(argc - L - 1 > 1){
      write(STDOUT_FILENO,argv[i],strlen(argv[i]));
      write(STDOUT_FILENO,":\n",2);
    }
    if(!strcmp(argv[i],"-l")) continue;
    char *p = getRealPath(argv[i]);
    special_path sp = special_path_maker(p);
    free(p);
    if(strlen(sp.tar_path) > 0) sp.tar_path[strlen(sp.tar_path) - 1] = '\0';

    /* le tar a ouvrir est a l'adresse "/" + pwd + nom du tar */
    char tarlocation[strlen(sp.path) + strlen(sp.tar_name) + 2];
    memset(tarlocation,0,strlen(sp.path) + strlen(sp.tar_name) + 2);
    sprintf(tarlocation,"/%s%s",sp.path,sp.tar_name);

      /* si on est dans un tar */
    if(strlen(sp.tar_name) > 0){

        if(strlen(sp.tar_path) > 0){
          struct posix_header *ph = getHeader(tarlocation,sp.tar_path);

          if(ph == NULL){
	    free(ph);
            perror("impossible d'ouvrir le fichier.\n");
            return -1;
          }
          // Si c'est un fichier on print son nom
          if(ph->typeflag == '0'){
	    free(ph);
            write(STDOUT_FILENO,argv[i],strlen(argv[i]));
            write(STDOUT_FILENO,"\n",1);
            continue;
          }
        }
        
        ls_tar(tarlocation,sp.tar_path,L);
      }else{
        int r;
        r = fork();
        switch(r){
          case -1 : return -1;
          case 0 :
          if(!L){
            execlp("ls","ls",tarlocation,NULL);
          }else{
            execlp("ls","ls","-l",tarlocation,NULL);
          }
          exit(-1);
          default : waitpid(r,NULL,0); break;
        }
    }
    if(argc - L > 1 && i < argc - 1) write(STDOUT_FILENO,"\n",1);     
    freeSpecialPath(sp);
    }
    return 0;
  }

int optionL(int argc, char **argv){//if option<-1 then "ls" else "ls -l"
  for(int i=0;i<argc;i++){
    if(!strcmp(argv[i],"-l")) return 1;
  }
  return 0;
}

int sameLevel(char *path){
  char *s = strchr(path,'/');
  if(s == NULL || s[1] == '\0') return 1;
  return 0;
}

int printOptionL(struct posix_header *tampon){
  if(tampon == NULL) return -1;

  unsigned long int int_time=strtol(tampon->mtime,NULL,8);
  const time_t time = (time_t)int_time;
  struct tm *date=gmtime(&time);
  unsigned int int_gid=strtol(tampon->gid,NULL,0);
  unsigned int int_uid=strtol(tampon->uid,NULL,0);
  struct group *gid=getgrgid(int_gid);//to get the gid name because info.st_gid only gives us an int
  struct passwd *uid=getpwuid(int_uid);//same
  unsigned int int_filesize=strtol(tampon->size,NULL,0);
  if(tampon->typeflag-48==5)int_filesize=4096;//Standard size for folder
  int line_size=10+2+strlen(uid->pw_name)+strlen(gid->gr_name)+3+2+3+2+strlen(tampon->name)+9+10;
  char line[line_size];
  memset(line,0,line_size);
  char mode[11];
  convert_stmode(tampon,mode);
  if(snprintf(line,sizeof(line),"%s %s %s %s %*d %s. %02d %02d:%02d %s", //we write the data from stat on the line
  mode,"1",uid->pw_name,gid->gr_name,5,
  int_filesize,month[date->tm_mon],date->tm_mday,date->tm_hour,date->tm_min,tampon->name)<0)return-1;
  if(write(1,line,sizeof(line))<0)return -1;
  if(write(1,"\n",1)<0)return -1;

  return 1;
}

int ls_tar(char *tarname, char *tarpath, int option){
  int fd = open(tarname,O_RDONLY);
  if(fd < 0) return -1;
  char path[strlen(tarpath) + 2];
  memset(path,0,strlen(tarpath) + 2);
  if(strlen(tarpath) > 0) sprintf(path,"%s/",tarpath);
  if(strlen(tarpath) > 0 && path[strlen(path) - 2] == '/')
    path[strlen(path) - 1] = '\0';
  int printed = 0;

  while(1){

    struct posix_header tampon;
    if(read(fd, &tampon, sizeof(struct posix_header)) < 0) return -1;

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    /* same prefix */
    if(!strncmp(tampon.name,path,strlen(path))){
      /* are on 'same level' (and not the same)*/
      if(strcmp(tampon.name,path) != 0 && sameLevel(tampon.name + strlen(path))){
        printed++;
        if(!option){
          if(write(1,tampon.name + strlen(path),strlen(tampon.name) - strlen(path)) < 0)return -1;
          if(write(1," ",1) < 0)return -1;
        }else{
          // TODO : MISSING TOTAL
          /* on retire le 'prefix' */
          char newname[100];
          memset(newname,0,100);
          strcat(newname,tampon.name + strlen(path));
          memcpy(tampon.name,newname,100);

          printOptionL(&tampon);
        }
      }
    }

    /* we get the size of the file for this header */
    unsigned int filesize;
    sscanf(tampon.size,"%o", &filesize);

    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;
    lseek(fd,s * 512,SEEK_CUR);
  }
  if(!option && printed > 0) if(write(1,"\n",1) < 0) return -1;
  return 0;
}


int nbDigit (int n) {//return the number of digit of a certain integer. We need it for info.st_size
    if (n == 0) return 1;
    return floor (log10 (abs (n))) + 1;
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
int main(int argc,char **argv){
  return ls(argc,argv);
}
