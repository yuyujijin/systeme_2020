#include "ls.h"
#include "tar_manipulation.h"
#include "tar.h"
int ls(char *const args[],int argc){
  int fork_ls=fork();
  switch (fork_ls) {
    case -1/* value */:perror("fork");exit(EXIT_FAILURE);
    case 0 :
      {
        int option=has_option(args,argc);
        int *tarIndex=has_Tar(args,argc);
        for(int i=0;i<argc;i++){
          if(tarIndex[i]==0&&execlp("ls","ls",args[i],NULL)<0){
            perror("ls");
            exit(EXIT_FAILURE);
            return -1;
          }else if(tarIndex[i]==1){
            ls_tar(args[i],option);
          }
        }
        exit(EXIT_SUCCESS);
        break;
      }
    default : wait(NULL);
  }
  return 0;
}

int has_option(char *const args[],int argc){
  for(int i=0;i<argc;i++){
    if(strcmp(args[i],"-l")==0)return 1;
  }
  return 0;
}

int ls_tar(const char *args,int option){//if option==0 then "ls" else if option==0 "ls -l"
  struct posix_header* posix_header=posix_header_from_tarFile(args);
  if(posix_header==NULL)return -1;
  return 0;
}
int ls_tar_option(struct posix_header* posix_header){
  return 0;

}
// int main(int argc, char const *argv[]) {
//   printf("%d\n",isTar("."));
//   return 0;
// }
