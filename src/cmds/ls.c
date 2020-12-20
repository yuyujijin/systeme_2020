#include "ls.h"

char *month[]={"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

int ls(int argc, char **argv){
  if(argc == 1) return ls_tar(0);
  int L = optionL(argc,argv);

  switch (fork()) {
    case -1 : perror("fork");exit(EXIT_FAILURE);
    case 0 :
    for(int i = 1; i < argc; i++){
      if(!strcmp(argv[i],"-l")) continue;
      if(argc > 2){
        char name[256];
        memset(name,0,256);
        sprintf(name,"%s:\n",argv[i]);
        write(STDOUT_FILENO,name,strlen(name));
      }
      char *last_arg;
      switch (fork()) {
        case -1 : perror("fork");exit(EXIT_FAILURE);
        case 0 :
        // on tente d'acceder au chemin, sans le dernier argument
        // si il est non accessible -> erreur
        // ensuite, on verifie si le dernier arg existe dans le contexte actuel
        // sinon, erreur ->
        // puis on essaie d'y acceder, si ça fonctionne, c'est un dossier
        // sinon c'est un dossier
        last_arg = getLastArg(argv[i]);
        // si le dernier argument != argv[i] (juste un fichier simple)
        if(strcmp(argv[i],last_arg)) if(cd(pathminus(argv[i],last_arg)) < 0) exit(EXIT_FAILURE);
        // on vérifie si le fichier existe (dans les 2 contextes)
        if(strlen(getenv("TARNAME"))){
          if(!existsTP(last_arg)) exit(EXIT_FAILURE);
        }else{
          int fd = open(last_arg,O_RDONLY);
          if(fd < 0) exit(EXIT_FAILURE);
          close(fd);
        }
        // si on peut y acceder
        if(cd(last_arg)){
          if(strlen(getenv("TARNAME"))) return ls_tar(L);
          if(L) execlp("ls","ls","-l",NULL);
          execlp("ls","ls",NULL);
        }
        // sinon on print le nom (non dossier)
        write(STDOUT_FILENO,last_arg,strlen(last_arg));
        write(STDOUT_FILENO,"\n",1);
        exit(1);
        default : wait(NULL); break;
      }
      if(i < argc - 1) write(STDOUT_FILENO,"\n",1);
    }
    exit(0);
    default : wait(NULL);break;
  }
  return 0;
}

int optionL(int argc, char **argv){//if option<-1 then "ls" else "ls -l"
  for(int i=0;i<argc;i++){
    if(!strcmp(argv[i],"-l")) return 1;
  }
  return 0;
}

int ls_tar(int option){
  struct posix_header** posix_header = posix_header_from_tarFile(getenv("TARNAME"),getenv("TARPATH"));
  if(posix_header == NULL) return -1;
  // normal case
  if(option != 1){
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
int main(int argc,char **argv){
  return ls(argc,argv);
}
