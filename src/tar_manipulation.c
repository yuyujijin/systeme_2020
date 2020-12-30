#define _POSIX_C_SOOURCE 200112L
#define _XOPEN_SOURCE 500
#include "tar_manipulation.h"

int addTar(const char *path, const char *name, char typeflag){
  int fd;

  fd = open(path,O_WRONLY);
  if(fd < 0){ perror("tar introuvable.\n"); return -1; }

  if(!isTar(path)){ perror("tar innaccessible.\n"); return -1; }

  /* we get the offset right before the empty blocks */
  size_t offt = offsetTar(path);
  /* and we go there */
  lseek(fd,offt + BLOCKSIZE,SEEK_CUR);

  /* We use a bufer of size BLOCKSIZE bytes, that reads in STDIN while it can */
  char buffer[BLOCKSIZE];
  unsigned int bufsize = 0;
  /* Put it at '\0' on every bytes, in case we didnt read 512 bytes */
  memset(buffer,'\0',BLOCKSIZE);
  /* if we arent writing an empty file */
  if(typeflag != '5'){
    size_t size;
    /* read everything from STDIN and write in the tarball */
    while((size = read(STDIN_FILENO, buffer, BLOCKSIZE)) > 0){
      bufsize += size;
      if(write(fd,buffer,size) < 0) return -1;
    }
    /* if the last red block is < BLOCKSIZE then we have to fill with '\0' */
    if(size < BLOCKSIZE){
      char empty[BLOCKSIZE - size];
      memset(empty,'\0',BLOCKSIZE - size);
      if(write(fd,empty,BLOCKSIZE - size) < 0) return -1;
    }

    /* We then put the two empty blocks at the end of the tar */
    char emptybuf[512];
    memset(emptybuf,0,512);
    for(int i = 0; i < 2; i++){ if(write(fd, emptybuf,512) < 0){perror("erreur lors de l'écriture dans le tar \n"); return -1; } }
  }

  /* we put ourselves just before the blocks we've written */
  lseek(fd, offt,SEEK_SET);

  /* Now we write the header */
  struct posix_header hd;
  memset(&hd,'\0',sizeof(struct posix_header));

  memcpy(hd.name, name, strlen(name) + 1);
  sprintf(hd.mode,"0000664");

  sprintf(hd.size, "%011o", bufsize);

  hd.typeflag = typeflag;
  memcpy(hd.magic,"ustar",5);
  memcpy(hd.version,"00",2);
  set_checksum(&hd);

  if(check_checksum(&hd) < 0) return -1;

  if(write(fd, &hd, sizeof(struct posix_header)) < 0){perror("erreur lors de l'écriture dans le tar \n"); return -1; }

  close(fd);

  return 1;
}
int appendTar(char *path, char *name){
  // On réecrit le fichier à la fin
  // puis on supprimer 'l'original' et on lit depuis STDIN et écrit dans le fichier
  // et on met à jour la taille du header à la fin
  struct posix_header tampon;
  int fd, s;
  unsigned int filesize;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDWR);
  if(fd < 0){ perror("tar introuvable.\n"); return -1; }

  while(1){
    /* create the buffer to read the header */
    if(read(fd, &tampon, sizeof(struct posix_header)) <= 0)
    { perror("erreur lors de la lecture du tar.\n"); return -1; }

    /* if its empty, we then just addTar */
    if(isEmpty(&tampon)){ close(fd); return addTar(path,name,'0'); }

    /* we get the size of the file for this header */
    sscanf(tampon.size,"%o", &filesize);

    /* and size of its blocs */
    s = (filesize + 512 - 1)/512;

    if(strcmp(tampon.name,name) == 0) break;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }


  int pipefd[2];
  pipe(pipefd);

  char buf[BLOCKSIZE];
  memset(buf,0,BLOCKSIZE);

  // Ce sont les bytes qu'on doit rajouter au bloc final du fichier
  // avant de réecrire de nouveaux blocs
  size_t sizeread;

  switch(fork()){
    case -1 : return -1;
    case 0 :
    close(pipefd[0]);
    for(int i = 0; i < s; i++){
      memset(buf,0,BLOCKSIZE);
      if(read(fd,buf,BLOCKSIZE) < 0)
      { perror("erreur lors de la lecture dans le tar."); return -1; }
      if(write(pipefd[1],buf,BLOCKSIZE) < 0)
      { perror("erreur lors de l'écriture dans le tar."); return -1; }
    }
    close(fd);
    close(pipefd[1]);
    exit(0);
    default :
    close(pipefd[1]);
    close(fd);

    fd = open(path,O_RDWR);
    if(fd < 0){ perror("tar introuvable.\n"); return -1; }

    lseek(fd, offsetTar(path), SEEK_SET);
    if(write(fd,&tampon,sizeof(struct posix_header)) < 0)
    { perror("erreur lors de l'écriture dans le tar."); return -1; }

    while((sizeread = read(pipefd[0], buf, BLOCKSIZE)) > 0){
      if(write(fd,buf,sizeread) < 0)
      { perror("erreur lors de l'écriture dans le tar."); return -1; }
    }
    close(pipefd[0]);
    break;
  }

  int missing = BLOCKSIZE - strlen(buf);

  // Puis on supprimer le fichier
  if(rmTar(path,name) < 0)
  { perror("appendTar."); return -1; }

  lseek(fd,offsetTar(path) - missing,SEEK_SET);

  char buffer[BLOCKSIZE];
  unsigned int bufsize = 0;
  /* Put it at '\0' on every bytes, in case we didnt read 512 bytes */
  memset(buffer,'\0',BLOCKSIZE);
  /* if we arent writing an empty file */
  size_t size;
  size_t sizeToRead = missing;
  /* read everything from STDIN and write in the tarball */
  while((size = read(STDIN_FILENO, buffer, sizeToRead)) > 0){
    bufsize += size;
    sizeToRead -= size;
    if(write(fd,buffer,size) < 0){ perror("erreur lors de l'écriture dans le tar \n"); return -1; }
    memset(buffer,0,BLOCKSIZE);
    if(sizeToRead <= 0) sizeToRead = BLOCKSIZE;
  }

  /* if the last red block is < BLOCKSIZE then we have to fill with '\0' */
  if(size - sizeToRead < BLOCKSIZE){
    char empty[BLOCKSIZE - size + sizeToRead];
    memset(empty,'\0',BLOCKSIZE - size + sizeToRead);
    if(write(fd,empty,BLOCKSIZE - size + sizeToRead) < 0) return -1;
  }

  /* We then put the two empty blocks at the end of the tar */
  char emptybuf[512];
  memset(emptybuf,0,512);
  for(int i = 0; i < 2; i++){ if(write(fd, emptybuf,512) < 0){ perror("erreur lors de l'écriture dans le tar \n"); return -1; } }

  // Puis on modifie la taille du fichier (son filesize)
  lseek(fd,0,SEEK_SET);

  while(1){
    /* create the buffer to read the header */
    if(read(fd, &tampon, sizeof(struct posix_header)) <= 0)
    { perror("erreur lors de la lecture du tar.\n"); return -1; }

    /* if its empty, we stop */
    if(isEmpty(&tampon))
    { perror("Fichier introuvable.\n"); return -1; }

    /* we get the size of the file for this header */
    sscanf(tampon.size,"%o", &filesize);

    /* and size of its blocs */
    s = (filesize + 512 - 1)/512;

    if(strcmp(tampon.name,name) == 0) break;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }

  // On retourne juste devant le header, pour l'écraser
  lseek(fd, - BLOCKSIZE, SEEK_CUR);

  sprintf(tampon.size, "%011o", bufsize + filesize);
  set_checksum(&tampon);
  if(check_checksum(&tampon) < 0) return -1;

  if(write(fd,&tampon,BLOCKSIZE) < 0)
  { perror("erreur lors de l'ecriture dans le tar.\n"); return -1; }

  return 1;
}


struct posix_header* getHeader(const char *path, const char *name){
  struct posix_header* tampon = malloc(sizeof(struct posix_header));
  int fd, s;
  unsigned int filesize;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0){ perror("impossible d'ouvrir le tar.\n"); return NULL; }

  while(1){
    /* create the buffer to read the header */
    if(read(fd, tampon, sizeof(struct posix_header)) <= 0)
    { perror("erreur lors de la récuperation du header.\n"); return NULL; }

    /* if its empty, we stop */
    if(isEmpty(tampon)) return NULL;

    /* we get the size of the file for this header */
    sscanf(tampon->size,"%o", &filesize);

    /* and size of its blocs */
    s = (filesize + 512 - 1)/512;

    if(strcmp(name,tampon->name) == 0) return tampon;
    /* si c'est un dossier, on vérifie sans le '/' */
    if(tampon->typeflag == '5' && strncmp(name,tampon->name,
      (strlen(tampon->name) > strlen(name))?
      strlen(tampon->name) - 1 : strlen(name)) == 0) return tampon;


    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }

  close(fd);

  return NULL;
}

int rdTar(const char *path, const char *name){
  struct posix_header tampon;
  int fd, s;
  unsigned int filesize;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0){ perror("tar introuvable.\n"); return -1; }

  while(1){
    /* create the buffer to read the header */
    if(read(fd, &tampon, sizeof(struct posix_header)) <= 0)
    { perror("erreur lors de la lecture du tar.\n"); return -1; }

    /* if its empty, we stop */
    if(isEmpty(&tampon))
    { perror("Fichier introuvable.\n"); return -1; }

    /* we get the size of the file for this header */
    sscanf(tampon.size,"%o", &filesize);

    /* and size of its blocs */
    s = (filesize + 512 - 1)/512;

    if(strcmp(tampon.name,name) == 0) break;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }

  char rd_buf[BLOCKSIZE];

  for(int i = 0; i < s; i++){
    int size;
    if((size = (read(fd,rd_buf,BLOCKSIZE))) < 0)
    { perror("erreur lors de la lecture du tar.\n"); return -1; }
    if(filesize < BLOCKSIZE) size = filesize;
    if((write(STDOUT_FILENO,rd_buf,size)) < 0)
    { perror("erreur lors de la lecture du tar.\n"); return -1; }
    filesize -= BLOCKSIZE;
  }

  close(fd);

  return 1;
}

int rmTar(const char *path, const char *name){
  struct posix_header tampon;

  int fd;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0){ perror("erreur lors de l'ouverture du tar.\n"); return -1; }

  int s = 0;
  int offset = 0;

  while(1){
    /* create the buffer to read the header */

    /*size_t size =*/ read(fd, &tampon, sizeof(struct posix_header));

    /* if its empty, we stop */
    if(isEmpty(&tampon)) return 0;

    /* if checksum fails, then its not a proper tar */
    /* we get the size of the file for this header */
    unsigned int filesize;
    sscanf(tampon.size,"%o", &filesize);

    /* and size of its blocs */
    s = (filesize + 512 - 1)/512;

    if(strcmp(tampon.name,name) == 0) break;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
    offset += (s+1) * BLOCKSIZE;
  }

  offset = (offset > 0)? offset: 0;
  /* We're then gonna use a pipe in order to move data */
  int rd_offset = offset + (1 + s) * BLOCKSIZE;

  close(fd);

  int fd_pipe[2];
  pipe(fd_pipe);

  char rd_buf[BLOCKSIZE];
  // memset(rd_buf,'\0',BLOCKSIZE);

  switch(fork()){
    case -1: perror("rmTar.\n"); return -1;
    /* son, reading data in file, writing them in the pipe */
    case 0:
    fd = open(path, O_RDONLY);
    if(fd < 0){ perror("erreur lors de l'ouverture du tar.\n"); return -1; }
    lseek(fd,rd_offset,SEEK_SET);
    /* no need to read in the pipe */
    close(fd_pipe[0]);

    while((read(fd, rd_buf, BLOCKSIZE)) > 0){
      if(write(fd_pipe[1], rd_buf, BLOCKSIZE) < 0)
      { perror("erreur lors de l'ecriture dans le tar.\n"); return -1; }
      memset(rd_buf,'\0',BLOCKSIZE);
    }
    close(fd_pipe[1]);
    close(fd);

    /* the son leaves here */
    exit(EXIT_SUCCESS);

    /* father, reading data in the pipe, writing them in the file */
    default:
    fd = open(path,O_WRONLY);
    if(fd < 0)
    { perror("erreur lors de l'ouverture du tar.\n"); return -1; }
    lseek(fd,offset,SEEK_SET);

    /* no need to write in the pipe */
    close(fd_pipe[1]);

    while((read(fd_pipe[0], rd_buf, BLOCKSIZE)) > 0){
      if(write(fd, rd_buf, BLOCKSIZE) < 0)
      { perror("erreur lors de l'ecriture dans le tar.\n"); return -1; }
      memset(rd_buf,'\0',BLOCKSIZE);
    }
    close(fd_pipe[0]);
    break;
  }

  /* We must now truncate the end of the file (remove empty blocks left) */
  int tarsize = lseek(fd, 0, SEEK_END);
  if(ftruncate(fd, tarsize - (1 + s) * BLOCKSIZE) < 0)
  { perror("rmTar.\n"); return -1; }
  close(fd);
  return 1;

}

int isEmpty(struct posix_header* p){
  for(int i = 0; i < BLOCKSIZE; i++) if((p->name)[i] != '\0') return 0;
  return 1;
}

int isTar(const char* path){

  struct posix_header tampon;
  int fd;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0)
  { perror("erreur lors de l'ouverture du tar.\n"); return -1; }

  /* issue where tar made with 'tar cvf ...' arent recognized as tar */
  /* little hotfix for now */
  return (strstr(path,".tar") != NULL);

  while(1){
    /* create the buffer to read the header */

    read(fd, &tampon, sizeof(struct posix_header));

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    /* if checksum fails, then its not a proper tar */
    if(check_checksum(&tampon) == 0) return 0;

    /* we get the size of the file for this header */
    unsigned int filesize;
    sscanf(tampon.size,"%o", &filesize);

    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }

  close(fd);

  return 1;
}

size_t offsetTar(const char *path){
  int fd;
  int offset = 0;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0)
  { perror("erreur lors de l'ouverture du tar.\n"); return -1; }

  struct posix_header buf;
  size_t size;

  while((size = read(fd, &buf, BLOCKSIZE)) > 0){
    /* we get the size of the file for this header */
    if(isEmpty(&buf)){ close(fd); return offset; }

    unsigned int filesize;
    sscanf(buf.size,"%o", &filesize);

    /* and size of its blocs */
    int s = (filesize + BLOCKSIZE - 1)/BLOCKSIZE;
    offset += BLOCKSIZE * (s + 1);

    lseek(fd,s * BLOCKSIZE, SEEK_CUR);
  }

  close(fd);

  return offset;
}


void has_Tar(char *const args[],int argc,int *tarIndex){
  for(int i=0;i<argc;i++){
    if(strstr(args[i],".tar")){//just check that the path has a tar in it
      tarIndex[i]=1;
    }
  }
}

int has_tar(const char* argv)
{
  //loop until |argv|-5 if we want create a
  //directory "xxx.tar"
  if(strlen(argv)<5) return 0;
  for(unsigned i=0;i<strlen(argv)-5;i++)
    {
      if(argv[i]=='.' && argv[i+1]=='t' && argv[i+2]=='a' && argv[i+3]=='r'){
	//check if the path is a directory name "xxx.tar" or just
	//a tar
	int fd=open(substr(argv,0,i+5),O_DIRECTORY);
	if (fd<0){
	  close(fd);
	  //return index of path after "XX.tar/"
	  return i+5;
	}
      }
    }
  return 0;
}

char* get_tar_from_full_path(const char * path){
  char *subpath=strstr(path,".tar");
  if(subpath==NULL)return NULL;
  int size=strlen(path)-strlen(subpath)+4;
  char *result=malloc(size+1);
  strncpy(result,path,size);
  result[size]='\0';
  return result;
}

struct posix_header** posix_header_from_tarFile(char *tarname, char *path){
  int fd = open(tarname,O_RDONLY);
  if(fd < 0) return NULL;

  int i = 0;
  struct posix_header** result = NULL;//only 1 header in the beginning

  while(1){

    struct posix_header *tampon = malloc(sizeof(struct posix_header));
    if(read(fd, tampon, sizeof(struct posix_header)) < 0) return NULL;

    /* if its empty, we stop */
    if(isEmpty(tampon)) break;

    /* same prefix */
    if(!strncmp(tampon->name,path,strlen(path))){
      /* we only add file with <= 1 '/' in their name */
      /* no '/' */
      if(strchr(tampon->name + strlen(path),'/') == NULL){
        result = realloc(result,sizeof(struct posix_header*) * (i + 2));
        result[i++] = tampon;
      }else{

      }
    }

    /* we get the size of the file for this header */
    unsigned int filesize;
    sscanf(tampon->size,"%o", &filesize);

    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;
    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }
  result[i] = NULL;

  close(fd);
  return result;
}

int existsTP(char *filename){
  //error if we're not on a tar
  if(!strlen(getenv("TARNAME"))) return -1;

  //if filename == . or ..
  if(!strcmp(filename,".") || !strcmp(filename,"..")) return 1;
  
  char s[strlen(filename) + strlen(getenv("TARPATH")) + 1];
  memset(s,0,strlen(filename) + strlen(getenv("TARPATH")) + 1);
  strcat(s,getenv("TARPATH"));
  //if(strlen(s)) strcat(s,"/");
  strcat(s,filename);
  
  return exists(getenv("TARNAME"),s);
}

int exists(char *tarpath, char *filename){
  if(!strcmp(filename,".") || !strcmp(filename,"..")) return 1;

  struct posix_header tampon;
  int fd;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(tarpath,O_RDONLY);
  if(fd < 0)
  { perror("erreur lors de l'ouverture du tar.\n"); return -1; }

  /* if we just check tarpath */
  if(strlen(filename) == 0) return 1;

  while(1){
    /* create the buffer to read the header */
    read(fd, &tampon, sizeof(struct posix_header));

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    if(strcmp(filename,tampon.name) == 0) return 1;
    /* si c'est un dossier, on vérifie sans le '/' */
    if(tampon.typeflag == '5' && strncmp(filename,tampon.name,
      (strlen(tampon.name) > strlen(filename))?
      strlen(tampon.name) - 1 : strlen(filename)) == 0) return 1;

    /* we get the size of the file for this header */
    unsigned int filesize;
    sscanf(tampon.size,"%o", &filesize);

    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }

  close(fd);

  return 0;
}

int is_source(const char* path){
  return (strcmp(path+strlen(path)-4,strstr(path,".tar"))==0||strcmp(path+strlen(path)-5,strstr(path,".tar/"))==0);
}

char *substr(const char *src,int start,int end) {
  char *dest=NULL;
  if (end-start>0) {
    dest = malloc(end-start+1);
    memset(dest,'\0',end-start+1);
    if(dest==NULL) perror("");
    for(int i=0;i<end-start;i++){
      dest[i]=src[start+i];
    }
    //dest[end-start]='\0';
  }
  return dest;
}

int file_exists_in_tar(char* path, char* name){
  struct posix_header hd;
  int fd;

  fd = open(path,O_RDONLY);
  //if tarball path doesn't exist
  if(fd<0)
    {
      return 0;
    }
  
  while(read(fd, &hd, sizeof(struct posix_header))){
    if(hd.name[0]=='\0')
      return 0;
    if(strcmp(name,hd.name)==0)  { close(fd); return 1;}

    unsigned int filesize;
    sscanf(hd.size,"%o", &filesize);
    int s = (filesize + 512 - 1)/512;
    lseek (fd, s* BLOCKSIZE, SEEK_CUR);
  }
  return 0;
}

int is_empty (char *tarpath, char *name)
{
  struct posix_header hd;
  int fd;

  fd = open(tarpath,O_RDONLY);

  if(fd<0)
    {
      close (fd);
      return -1;
    }

  //count of nb of files path/name/xxx
  // if > 1 we can't erase dir, it's not empty
  unsigned int count = 0;
  while(read(fd, &hd, sizeof(struct posix_header))){
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
	    close (fd);
	    return 0;
	  }
      }
    int filesize;
    sscanf(hd.size,"%d", &filesize);
    int s = (filesize + 512 - 1)/512;
    struct posix_header* temp = malloc(sizeof(struct posix_header) * s);
    read(fd, temp, s * BLOCKSIZE);
    free(temp);
  }
  close (fd);
  return 1;
}

//return 1 is the par path/ is empty
//else 0
int is_empty_tar (char* path)
{
  int fd = open(path, O_RDONLY);
  struct posix_header hd;

  if(fd<0)
    {
      close (fd);
      return -1;
    }

  int ret = 1;
  if (read(fd, &hd, sizeof(struct posix_header)))
    ret = (hd.name[0] == '\0');
  close(fd);
  return ret;

}
