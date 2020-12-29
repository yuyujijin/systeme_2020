#include "unit_test.h"

int cd_test_length = 3;
char *cd_test[] = {"test","test/../test/doggy.tar/test/bigWouf","test/doggy.tar/.."};
void cd_uni_test(){
  switch (fork()) {
    case -1:exit(-1);
    case 0:
      execlp("touch","touch","test/bigWouf",NULL);
      exit(1);
    default :
      wait(NULL);
      break;
  }
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("tar","tar","cvf","test/doggy.tar","test/bigWouf",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }
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


char *ls_test[] = {"cmds/ls","-l","test/mammouth.tar/test",NULL};
void ls_uni_test(){
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("tar","tar","cvf","test/mammouth.tar","test",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }
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

//TODO bug in cp, it's more of a syntax bug, the root of the bug is cdTo()
char *cp_test[] = {"cmds/cp","-r","test/biggyWallas.tar","test/Popeye",NULL};
void cp_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
    switch (fork()) {
      case -1:exit(-1);
      case 0:
      execlp("tar","tar","cvf","test/biggyWallas.tar","test",NULL);
      exit(1);
      default :
        wait(NULL);
        break;
    }
      switch (fork()) {
        case -1 : return;
        case 0 :
          execvp("cmds/cp",cp_test);
          exit(1);
          default :
          wait(NULL);
          exit(1);
      }
    default :
    wait(NULL);
    break;
  }
}

//TODO bug in cat can't access file inside test/ we should use cd in it
char *cat_test[] = {"cmds/cat","test/catTest.tar/cat",NULL};
void cat_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
    switch (fork()) {
      case -1 : return;
      case 0 :
        switch (fork()) {
          case -1:exit(-1);
          case 0:
            {
              int fd=open("test/cat",O_WRONLY|O_CREAT|O_TRUNC,0644);
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
          execlp("tar","tar","cvf","test/catTest.tar","test/cat",NULL);
          exit(1);
          default :
            wait(NULL);
            break;
        }
        execvp("cmds/cat",cat_test);
        exit(1);
      default :
      wait(NULL);
      exit(1);
    }
    default :
    wait(NULL);
    break;
  }
}

char *rmdir_test[] = {"cmds/rmdir","test/emptyRmdir.tar/test/crmdir","test/emptyRmdir.tar","test/rmdir.tar/test/armdir",NULL};
void rmdir_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      switch (fork()) {
        case -1 : return;
        case 0 :
        switch (fork()) {
          case -1:exit(-1);
          case 0:
          execlp("rm","rm","-r","test/crmdir",NULL);
          exit(1);
          default :
            wait(NULL);
            break;
        }
        switch (fork()) {
          case -1:exit(-1);
          case 0:
          execlp("mkdir","mkdir","test/armdir",NULL);
          exit(1);
          default :
            wait(NULL);
            break;
        }
        switch (fork()) {
          case -1:exit(-1);
          case 0:
          execlp("mkdir","mkdir","test/armdir/brmdir",NULL);
          exit(1);
          default :
            wait(NULL);
            break;
        }
        switch (fork()) {
          case -1:exit(-1);
          case 0:
          execlp("mkdir","mkdir","test/crmdir",NULL);
          exit(1);
          default :
            wait(NULL);
            break;
        }
        switch (fork()) {
          case -1:exit(-1);
          case 0:
          execlp("tar","tar","cvf","test/rmdir.tar","test/armdir",NULL);
          exit(1);
          default :
            wait(NULL);
            break;
        }
        switch (fork()) {
          case -1:exit(-1);
          case 0:
          execlp("tar","tar","cvf","test/emptyRmdir.tar","test/crmdir",NULL);
          exit(1);
          default :
            wait(NULL);
            break;
        }
        execvp("cmds/rmdir",rmdir_test);
        //if rmdir.tar doesn't exist and emptyRmdir does after this then it is not working as it should
        exit(1);
        default :
        wait(NULL);
        exit(1);
      }
    default :
    wait(NULL);
    break;
  }
}

//TODO we need to cd to test cause we can't access the file inside test
char *mkdir_test[] = {"cmds/mkdir","test/new.tar","test/old.tar/test/a/b",NULL};
void mkdir_uni_test(){
  switch(fork()){
    case -1 : return;
    case 0 :
      switch (fork()) {
        case -1 : return;
        case 0 :
          switch (fork()) {
            case -1:exit(-1);
            case 0:
            execlp("mkdir","mkdir","test/a",NULL);
            exit(1);
            default :
              wait(NULL);
              break;
          }
          switch (fork()) {
            case -1:exit(-1);
            case 0:
            execlp("tar","tar","cvf","test/old.tar","test/a",NULL);
            exit(1);
            default :
              wait(NULL);
              break;
          }
          execvp("cmds/mkdir",mkdir_test);
          exit(1);
        default :
        wait(NULL);
        exit(1);
      }
    default :
    wait(NULL);
    break;
  }
}

int main(){
  switch (fork()) {
    case -1:exit(-1);
    case 0:
    execlp("mkdir","mkdir","test",NULL);
    exit(1);
    default :
      wait(NULL);
      break;
  }

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

}
