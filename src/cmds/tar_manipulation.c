#define _POSIX_C_SOOURCE 200112L
#define _XOPEN_SOURCE 500
#include "tar_manipulation.h"

int addTar(char *path, char *name/*, char typeflag*/){
  int fd;

  fd = open(path,O_WRONLY);
  if(fd < 0) return -1;

  if(!isTar(path)) return -1;

  /* we get the offset right before the empty blocks */
  size_t offt = offsetTar(path) - BLOCKSIZE;
  /* and we go there */
  lseek(fd,offt + BLOCKSIZE,SEEK_CUR);

  /* We use a bufer of size BLOCKSIZE bytes, that reads in STDIN while it can */
  char buffer[BLOCKSIZE];
  unsigned int bufsize = 0;
  /* Put it at '\0' on every bytes, in case we didnt read 512 bytes */
  memset(buffer,'\0',BLOCKSIZE);
  size_t size;
  /* read everything from STDIN and write in the tarball */
  while((size = read(1, buffer, BLOCKSIZE)) > 0){
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

  /* we put ourselves just before the blocks we've written */
  lseek(fd, offt,SEEK_SET);

  /* Now we write the header */

  struct posix_header hd;
  memset(&hd,'\0',sizeof(struct posix_header));

  memcpy(hd.name, name, strlen(name) + 1);
  sprintf(hd.mode,"0000700");

  sprintf(hd.size, "%011o", bufsize);

  hd.typeflag = 0;
  memcpy(hd.magic,"ustar",5);
  memcpy(hd.version,"00",2);
  set_checksum(&hd);

  if(check_checksum(&hd) < 0) return -1;

  if(write(fd, &hd, sizeof(struct posix_header)) < 0) return -1;;

  close(fd);

  return 1;
}

int rmTar(char *path, char *name){
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
    int filesize;
    sscanf(tampon.size,"%d", &filesize);

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
  if((p->name)[0] == '\0') return 1;
  return 0;
}

int isTar(char* path){
  struct posix_header tampon;
  int fd;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0) return 0;

  while(1){
    /* create the buffer to read the header */

    read(fd, &tampon, sizeof(struct posix_header));

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    /* if checksum fails, then its not a proper tar */
    if(check_checksum(&tampon) == 0) return 0;

    /* we get the size of the file for this header */
    int filesize;
    sscanf(tampon.size,"%d", &filesize);

    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;

    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);
  }

  close(fd);

  return 1;
}

size_t offsetTar(char *path){
  int fd;
  int offset = 0;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(path,O_RDONLY);
  if(fd < 0) return -1;

  char buf[BLOCKSIZE];
  size_t size;

  while((size = read(fd, &buf, BLOCKSIZE)) > 0){
    offset += 512;
    if(buf[0] == '\0') return offset;
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
struct posix_header** posix_header_from_tarFile(const char *path){
  int directory=0;
  char *directory_name;//won't be used if directory==0
  int source=is_source(path);
  char altpath[strlen(path)+2];//we do this cause we want path to be a.tar/b/c/ and not a.tar/b/c for the case of folder
  strcpy(altpath,path);
  strcat(altpath,"/");
  char *tar_path=get_tar_from_full_path(path);
  /*If directory==1 then we are returning all the file inside path(a directory) which is inside the tar */
  /*If file==1 then we are only returning the posix_header of a file with the path name*/
  /*If both are zero then we are returning all the file inside "a.tar/." */
  int fd=open(tar_path,O_RDONLY);
  if(fd<0)return NULL;
  int index=0;
  /* if the file doesnt exist (or cant be opened), then its not a tar */
  struct posix_header** result=NULL;//only 1 header in the beginning
  while(1){
    struct posix_header *tampon=malloc(sizeof(struct posix_header));
    if(read(fd, tampon, sizeof(struct posix_header))<0)return NULL;

    /* if its empty, we stop */
    if(isEmpty(tampon)) break;

    if(source==0&&(strcmp(tampon->name,strstr(path,".tar/")+5)==0||strcmp(tampon->name,strstr(altpath,".tar/")+5)==0)){
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // !!!!!!!! If you find a bug it might be here, the typeflag on my computer has a weird behavior !!!!!!!!
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      if(tampon->typeflag==53){//on my computer a directory has the value 53 weird ...
        directory_name=tampon->name;
        directory=1;
      }else if(tampon->typeflag==48){//we only want this file
        free(result);
        result=malloc(sizeof(struct posix_header));
        result[0]=tampon;
        return result;
      }
    }
    /* we get the size of the file for this header */
    int filesize;
    sscanf(tampon->size,"%d", &filesize);

    /* and size of its blocs */
    int s = (filesize + 512 - 1)/512;
    /* we read them if order to "ignore them" (we SHOULD use seek here) */
    char temp[s * BLOCKSIZE];
    read(fd, temp, s * BLOCKSIZE);

    //we check for every file such as directory_name/.../tampon->name and excludes the directorty itself
    //if the folder name is in the name basically //
    if(directory==1&&strcmp(directory_name,tampon->name)!=0&&strstr(tampon->name,directory_name)!=NULL){
      //now we check that we are in directory_name/tampon->name and not directory_name/...../tampon->name
      char *a=strstr(tampon->name,directory_name)+strlen(directory_name);//"a/b/c"->"b/c" if a is directory_name
      char *b=strrchr(tampon->name,'/')+1;//"a/b/c"->c if a is the directory_name and c is a file, if c is a directory "a/b/c/" -> '\0'
      if(strcmp(a,b)==0||b[0]=='\0'){
        result=realloc(result,sizeof(struct posix_header)+sizeof(result));
        if(result==NULL)return NULL;
        result[index]=tampon;
        index++;
      }
    }else if(source==1){
      //we only want the file(folder) like a.tar/b not a.tar/.../b
      if(strstr(tampon->name,"/")==NULL||strlen(strstr(tampon->name,"/"))==1){
        result=realloc(result,sizeof(struct posix_header)+sizeof(result));
        if(result==NULL)return NULL;
        result[index]=tampon;
        index++;
      }
    }
  }
  close(fd);
  return result;
}

int exists(char *tarpath, char *filename){
  struct posix_header tampon;
  int fd;

  /* if the file doesnt exist (or cant be opened), then its not a tar */
  fd = open(tarpath,O_RDONLY);
  if(fd < 0) return 0;

  while(1){
    /* create the buffer to read the header */
    size_t size = read(fd, &tampon, sizeof(struct posix_header));

    /* if its empty, we stop */
    if(isEmpty(&tampon)) break;

    if(strcmp(filename,tampon.name) == 0) return 1;

    /* we get the size of the file for this header */
    int filesize;
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
