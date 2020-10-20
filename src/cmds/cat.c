#include "cat.h"
#include "../tar_manipulation.h"
#include "../tar.h"
int cat(char *const args[],int argc){
  int fork_ls=fork();
  switch (fork_ls) {
    case -1/* value */:perror("fork");exit(EXIT_FAILURE);
    case 0 :
      {
        int* tarIndex=malloc(sizeof(int)*argc);
        has_Tar(args,argc,tarIndex);
        for(int i=1;i<argc;i++){//since args={"cat",args[1]....args[argc]} so we start with i=1
          //if we're working with a regular path
          if(tarIndex[i]==0){
            switch(fork()){
              case -1:perror("fork_loop");exit(EXIT_FAILURE);
              case 0 :
                if(args[i][0]!='-'&&execlp("cat","cat",args[i],NULL)<0){//args[i][0]!='-' is there to verify that we don't do cat -n and then cat -n file.txt but just cat -n file.txt
                  perror(args[i]);
                }
                exit(EXIT_SUCCESS);
                break;
              default :wait(NULL);break;
            }
          }
          //if we're working with tar
          else if(tarIndex[i]==1){
            if(cat_tar(args[i])<0){
              write(1,"cat:",29);
              write(1,args[i],sizeof(args[i]));
              write(1,": Aucun fichier ou dossier de ce type\n",39);
            }
          }
        }
        exit(EXIT_SUCCESS);
        break;
      }
    default : wait(NULL);break;
  }
  return 0;
}
int cat_tar(char *const arg){
  char *data=data_from_tarFile(arg);
  if(data==NULL)return -1;
  if(write(1,data,strlen(data)+1)<0)return -1;
  return 0;
}
int main(int argc,char *const argv[]){
  return cat(argv,argc);
}
