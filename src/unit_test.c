#include "unit_test.h"

void execute_cmd(char **argv){
  /* on récupère le path ou sont stockés les fonctions sur les tars */
  char *pathname = getenv("TARCMDSPATH");
  /* on y concatene la commande voulue */
  char pathpluscmd[strlen(pathname) + 1 + strlen(argv[0])];
  memset(pathpluscmd,0,strlen(pathname) + 1 + strlen(argv[0]));
  sprintf(pathpluscmd,"%s/%s",pathname,argv[0]);

  execv(pathpluscmd,argv);
}

int cd_test_length = 1;
char *cd_test[] = {"test/doggy.tar"};
void cd_uni_test(){
  for(int i = 0; i < cd_test_length; i++){
    switch(fork()){
      case -1 : return;
      case 0 :
        cd(cd_test[i]);
	char *pwds[2] = {"pwd",NULL};
	execute_cmd(pwds);
        exit(-1);
      default :
        wait(NULL);
        break;
    }
  }
}


char *ls_test[] = {"ls","-l","test/mammouth.tar/cmds",NULL};
void ls_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execute_cmd(ls_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

char *cp_test[] = {"cp","-r","test/bigWallas.tar","test/Popeye",NULL};
void cp_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execute_cmd(cp_test);
      exit(1);
    default :
      wait(NULL);
    break;
  }
}

char *cat_test[] = {"cat","test/catTest.tar/testCat/cat",NULL};
void cat_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execute_cmd(cat_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

char *rmdir_test[] = {"rmdir","test/rmdir.tar/testRmdir","test/rmdir.tar/cmds",NULL};
void rmdir_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execute_cmd(rmdir_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

char *mkdir_test[] = {"mkdir","test/new.tar","test/old.tar/test/",NULL};
void mkdir_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execute_cmd(mkdir_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}


char *rm_test[] = {"rm","-r","test/mugiwara.tar/cmds","test/mugiwara.tar",NULL};
void rm_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execute_cmd(rm_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}


char *mv_test[] = {"mv","test/gigaHallucinant.tar","test/omegaDestroyer",NULL};
void mv_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execute_cmd(mv_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

void createTestFolder(){
  switch(fork()){
  case -1 : return;
  case 0 : execlp("bash","bash","test_creator.sh",NULL); exit(-1);
  default : wait(NULL); break;
  }
}

#define MAX_SIZE 256

////MAIN/////
int main(){
  char cwd[MAX_SIZE];
  memset(cwd,0,MAX_SIZE);
  getcwd(cwd,MAX_SIZE);
  char tarcmdspath[strlen(cwd) + strlen("/cmds")];
  memset(tarcmdspath,0,strlen(cwd) + strlen("/cmds"));
  sprintf(tarcmdspath,"%s/cmds",cwd);

  setenv("TARCMDSPATH",tarcmdspath,1);
  setenv("TARPATH","",1);
  setenv("TARNAME","",1);

  createTestFolder();

  write(1,"\nTEST POUR CD : \n",strlen("\nTEST POUR CD : \n"));
  cd_uni_test();
  write(1,"\nTEST POUR LS : \n",strlen("\nTEST POUR LS : \n"));
  ls_uni_test();
  write(1,"\nTEST POUR CAT : \n",strlen("\nTEST POUR CAT : \n"));
  cat_uni_test();
  write(1,"\nTEST POUR CP : \n",strlen("\nTEST POUR CP : \n"));
  cp_uni_test();
  write(1,"\nTEST POUR RMDIR : \n",strlen("\nTEST POUR RMDIR : \n"));
  rmdir_uni_test();
  write(1,"\nTEST POUR MKDIR : \n",strlen("\nTEST POUR MKDIR : \n"));
  mkdir_uni_test();
  write(1,"\nTEST POUR RM : \n",strlen("\nTEST POUR RM : \n"));
  rm_uni_test();
  write(1,"\nTEST POUR MV : \n",strlen("\nTEST POUR MV : \n"));
  mv_uni_test();

}
