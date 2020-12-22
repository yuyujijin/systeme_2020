#include "ls.h"

char *month[]={"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

int ls(int argc, char **argv){
  if(argc == 1) return ls_tar(0);
  if(argc == 2 && !strcmp(argv[1],"-l")) return ls_tar(1);
  int L = optionL(argc,argv);

  
  switch (fork()) {
    case -1 : perror("fork");exit(EXIT_FAILURE);
    case 0 :
    for(int i = 1; i < argc; i++){
      if(!strcmp(argv[i],"-l")) continue;
      if(argc - L > 2)
	{
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
	  
	
printf("EHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
        last_arg = getLastArg(argv[i]);

printf("%s\n",last_arg);
        // si le dernier argument != argv[i] (juste un fichier simple)
        if(strcmp(argv[i],last_arg))
	  {
printf("%s",pathminus(argv[i],last_arg));
printf("%d\n",cd(pathminus(argv[i],last_arg)));
	  if(cd(pathminus(argv[i],last_arg)) < 0)
	    {
	      errno = ENOTDIR;
	      perror("ls");
	      exit(EXIT_FAILURE);
	    }
	  }
//printf("qjlqskdjbKJSDVNKQJSDBVLKSJVVMKSQJDMKSDJNKJDLKSDJB\n");
        // on vérifie si le dossier existe (dans les 2 contextes)
        if(strlen(getenv("TARNAME")))
	  {
	    if(existsTP(last_arg) <= 0)
	      {
		errno = ENOENT;
		perror("ls");
		exit(EXIT_FAILURE);
	      }
	  }
	else{
//printf("EEEEEEEEEEEHODOJSBFOLJZOHFHFOUHDIYFJRYDJYTFLUGLGILY\n");	  
          int fd = open(last_arg,O_RDONLY);
          if(fd < 0)
	    {
	      errno = EACCES;
	      perror("ls");
	      close(fd);
	      exit(EXIT_FAILURE);
	    }
	  close(fd);
        }
        // si on peut y acceder
        if(cd(last_arg) > 0){
//printf("EEEEEEEEEEEHODOJSBFOLJZOHFHFOUHDIYFJRYDJYTFLUGLGILY\n");
          if(strlen(getenv("TARNAME")))
	    return ls_tar(L);
          if(L) execlp("ls","ls","-l",NULL);
          execlp("ls","ls",NULL);
        }
//printf("EEEEEEEEEEEHODOJSBFOLJZOHFHFOUHDIYFJRYDJYTFLUGLGILY\n");
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

int ls_tar(int option){
  int fd = open(getenv("TARNAME"),O_RDONLY);
  char *path = getenv("TARPATH");
  if(fd < 0) return -1;

  while(1){

    struct posix_header tampon;
    if(read(fd, &tampon, sizeof(struct posix_header)) < 0) return -1;

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    /* same prefix */
    if(!strncmp(tampon.name,path,strlen(path))){
      /* are on 'same level' (and not the same)*/
      if(strcmp(tampon.name,path) != 0 && sameLevel(tampon.name + strlen(path))){
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
  if(!option) if(write(1,"\n",1) < 0) return -1;
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
