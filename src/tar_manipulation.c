#define _POSIX_C_SOOURCE 200112L
#define _XOPEN_SOURCE 500
#include "tar_manipulation.h"

int addTar(const char *path, const char *name, char typeflag){
  int fd;

  fd = open(path,O_WRONLY);
  if(fd < 0) return -1;

  if(!isTar(path)) return -1;

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
    for(int i = 0; i < 2; i++){ if(write(fd, emptybuf,512) < 0) return -1; }
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

  if(write(fd, &hd, sizeof(struct posix_header)) < 0) return -1;;

  close(fd);

  return 1;
}

struct posix_header* getHeader(const char *path, const char *name){
  struct posix_header* tampon = malloc(sizeof(struct posix_header));
  int fd, s;
  unsigned int filesize;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0) return NULL;

  while(1){
    /* create the buffer to read the header */
    if(read(fd, tampon, sizeof(struct posix_header)) <= 0) return NULL;

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
  if(fd < 0) return -1;

  while(1){
    /* create the buffer to read the header */
    if(read(fd, &tampon, sizeof(struct posix_header)) <= 0) return -1;

    /* if its empty, we stop */
    if(isEmpty(&tampon)) return -1;

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
    if((size = (read(fd,rd_buf,BLOCKSIZE))) < 0) return -1;
    if(filesize < BLOCKSIZE) size = filesize;
    if((write(STDIN_FILENO,rd_buf,size)) < 0) return -1;
    filesize -= BLOCKSIZE;
  }

  close(fd);

  return 1;
}

int rmTar(const char *path, const char *name){
  struct posix_header tampon;
  int fd;
  char err[256];

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0) return 0;

  int s = 0;
  int offset = 0;

  while(1){
    /* create the buffer to read the header */

    /*size_t size =*/ read(fd, &tampon, sizeof(struct posix_header));

    /* if its empty, we stop */
    if(isEmpty(&tampon)){
      sprintf(err,"Le fichier %s n'appartient pas à l'archive %s\n",name,path);
      write(1,err,strlen(err));
      return 0;
    };

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
    offset += (s + 1) * BLOCKSIZE;
  }

  offset = (offset > 0)? offset - BLOCKSIZE : 0;

  /* We're then gonna use a pipe in order to move data */

  int rd_offset = offset + (1 + s) * BLOCKSIZE;

  close(fd);

  int fd_pipe[2];
  pipe(fd_pipe);

  char rd_buf[BLOCKSIZE];
  memset(rd_buf,'\0',BLOCKSIZE);

  switch(fork()){
    case -1: return -1;
    /* son, reading data in file, writing them in the pipe */
    case 0:
    fd = open(path, O_RDONLY);
    if(fd < 0) return -1;
    lseek(fd,rd_offset,SEEK_SET);

    /* no need to read in the pipe */
    close(fd_pipe[0]);

    while((read(fd, rd_buf, BLOCKSIZE)) > 0){
      if(write(fd_pipe[1], rd_buf, BLOCKSIZE) < 0) return -1;
      memset(rd_buf,'\0',BLOCKSIZE);
    }
    close(fd_pipe[1]);
    close(fd);

    /* the son leaves here */
    exit(0);
    break;

    /* father, reading data in the pipe, writing them in the file */
    default:
    fd = open(path,O_WRONLY);
    if(fd < 0) return -1;
    lseek(fd,offset,SEEK_SET);

    /* no need to write in the pipe */
    close(fd_pipe[1]);

    while((read(fd_pipe[0], rd_buf, BLOCKSIZE)) > 0){
      if(write(fd, rd_buf, BLOCKSIZE) < 0) return -1;
      memset(rd_buf,'\0',BLOCKSIZE);
    }
    close(fd_pipe[0]);
    break;
  }

  /* We must now truncate the end of the file (remove empty blocks left) */
  int tarsize = lseek(fd, 0, SEEK_END);
  if(ftruncate(fd, tarsize - (1 + s) * BLOCKSIZE) < 0) return -1;
  close(fd);

  sprintf(err,"Le fichier %s a été supprimé avec succès de l'archive %s\n",name,path);
  write(1,err,strlen(err));

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
  if(fd < 0) return -1;

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
  if(fd < 0) return -1;

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

char* get_tar_from_full_path(const char * path){
  char *subpath=strstr(path,".tar");
  if(subpath==NULL)return NULL;
  int size=strlen(path)-strlen(subpath)+4;
  char *result=malloc(size+1);
  strncpy(result,path,size);
  result[size]='\0';
  return result;
}
char * data_from_tarFile(const char *path){
  char *tar_path=get_tar_from_full_path(path);
  int fd=open(tar_path,O_RDONLY);
  if(fd<0)return NULL;
  while(1){
    struct posix_header *tampon=malloc(sizeof(struct posix_header));
    if(read(fd, tampon, sizeof(struct posix_header))<0)return NULL;

    /* if its empty, we stop */
    if(isEmpty(tampon)) break;
    int filesize;
    sscanf(tampon->size,"%d", &filesize);
    if(strcmp(tampon->name,strstr(path,".tar/")+5)
       ==0&&tampon->typeflag==48){
      char *data=malloc(filesize+1);
      read(fd,data,filesize);
      data[filesize]='\0';
      return data;
    }
    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;
    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }
  return NULL;
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
  if(!strlen(getenv("TARNAME"))) return -1;
  if(!strcmp(filename,".") || !strcmp(filename,"..")) return 1;

  char s[strlen(filename) + strlen(getenv("TARPATH")) + 1];
  memset(s,0,strlen(filename) + strlen(getenv("TARPATH")) + 1);
  strcat(s,getenv("TARPATH"));
  if(strlen(s)) strcat(s,"/");
  strcat(s,filename);

  return exists(getenv("TARNAME"),s);
}

int exists(char *tarpath, char *filename){
  if(!strcmp(filename,".") || !strcmp(filename,"..")) return 1;

  struct posix_header tampon;
  int fd;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(tarpath,O_RDONLY);
  if(fd < 0) return -1;

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
