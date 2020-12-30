#include "unit_test.h"

int cd_test_length = 3;
char *cd_test[] = {"test/doggy.tar"};
void cd_uni_test(){
  for(int i = 0; i < cd_test_length; i++){
    switch(fork()){
      case -1 : return;
      case 0 :
        cd(cd_test[i]);
        execlp("cmds/pwd","pwd",NULL);
        exit(1);
      default :
        wait(NULL);
        break;
    }
  }
}


char *ls_test[] = {"cmds/ls","-l","test/mammouth.tar/cmds",NULL};
void ls_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execvp("cmds/ls",ls_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

char *cp_test[] = {"cmds/cp","-r","test/biggyWallas.tar","test/Popeye",NULL};
void cp_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execvp("cmds/cp",cp_test);
      exit(1);
    default :
      wait(NULL);
    break;
  }
}

char *cat_test[] = {"cmds/cat","test/catTest.tar/testCat/cat",NULL};
void cat_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execvp("cmds/cat",cat_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

char *rmdir_test[] = {"cmds/rmdir","test/rmdir.tar/testRmdir","test/rmdir.tar/cmds",NULL};
void rmdir_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execvp("cmds/rmdir",rmdir_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

char *mkdir_test[] = {"cmds/mkdir","test/new.tar","test/old.tar/test/",NULL};
void mkdir_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execvp("cmds/mkdir",mkdir_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}


char *rm_test[] = {"cmds/rm","-r","test/mugiwara.tar/cmds","test/mugiwara.tar",NULL};
void rm_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execvp("cmds/rm",rm_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}


char *mv_test[] = {"cmds/mv","test/gigaHallucinant.tar","test/omegaDestroyer",NULL};
void mv_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      execvp("cmds/mv",mv_test);
      exit(1);
    default :
    wait(NULL);
    break;
  }
}

void createTestFolder(){
  //TEST FOLDER
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("rm","rm","-r","-f","test",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }


  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("mkdir","mkdir","test",NULL);
    exit(1);
    default :
    wait(NULL);
    break;
  }


  //CD file
  switch (fork()) {
    case -1:exit(-1);
    case 0:
      execlp("tar","tar","cvf","test/doggy.tar","cmds",NULL);
      exit(1);
    default :
      wait(NULL);
      break;
  }

  //LS FILE
  switch (fork()) {
    case -1:exit(-1);
    case 0:
      execlp("tar","tar","cvf","test/mammouth.tar","cmds",NULL);
      exit(1);
    default :
      wait(NULL);
      break;
  }

  //CP FILE
  switch (fork()) {
    case -1:exit(-1);
    case 0:
      execlp("tar","tar","cvf","test/biggyWallas.tar","cmds",NULL);
      exit(1);
    default :
      wait(NULL);
      break;
  }

  //CAT FILE

  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("mkdir","mkdir","testCat",NULL);
    exit(1);
    default :
    wait(NULL);
    break;
  }
  switch (fork()) {
    case -1:exit(-1);
    case 0:
      {
        int fd=open("testCat/cat",O_WRONLY|O_CREAT|O_TRUNC,0644);
        if(fd<0)exit(-1);
        if(write(fd,"miaou miaouu",strlen("miaou miaouu"))<0)exit(-1);
        exit(1);
      }
    default :
      wait(NULL);
      break;
  }
  switch (fork()) {
    case -1:exit(-1);
    case 0:
      execlp("tar","tar","cvf","test/catTest.tar","testCat",NULL);
      exit(1);
    default :
      wait(NULL);
      break;
  }

  //RMDIR FILE
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("mkdir","mkdir","testRmdir",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("tar","tar","cvf","test/rmdir.tar","cmds","testRmdir",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }

  //MKDIR FILE
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("tar","tar","cvf","test/old.tar","cmds",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }

  //RM FILE
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("tar","tar","cvf","test/mugiwara.tar","cmds",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }

  //MV FILE
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("tar","tar","cvf","test/gigaHallucinant.tar","cmds",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }
}


////MAIN/////
int main(){
  char tarcmdspath[strlen(getcwd(NULL,0)) + strlen("/cmds")];
  memset(tarcmdspath,0,strlen(getcwd(NULL,0)) + strlen("/cmds"));
  sprintf(tarcmdspath,"%s/cmds",getcwd(NULL,0));

  setenv("TARCMDSPATH",tarcmdspath,1);

  createTestFolder();

  write(1,"\nTEST POUR CD : \n",strlen("\nTEST POUR CD : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      cd_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }

  write(1,"\nTEST POUR LS : \n",strlen("\nTEST POUR LS : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      ls_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }

  write(1,"\nTEST POUR CAT : \n",strlen("\nTEST POUR CAT : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      cat_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }

  write(1,"\nTEST POUR CP : \n",strlen("\nTEST POUR CP : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      cp_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }

  write(1,"\nTEST POUR RMDIR : \n",strlen("\nTEST POUR RMDIR : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      rmdir_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }

  write(1,"\nTEST POUR MKDIR : \n",strlen("\nTEST POUR MKDIR : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      mkdir_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }
  write(1,"\nTEST POUR RM : \n",strlen("\nTEST POUR RM : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      rm_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }
  write(1,"\nTEST POUR MV : \n",strlen("\nTEST POUR MV : \n"));
  switch (fork()) {
    case -1:return -1;
    case  0:
      setenv("TARNAME","",1);
      setenv("TARPATH","",1);
      mv_uni_test();
      exit(1);
    default:
      wait(NULL);
      break;
  }

}
