#include "cmds/cd.h"

int cd_test_length = 3;
char *cd_test[] = {"cmds/../test.tar","../././src/cmds/./..","test.tar/a/b/../b/c/./."};

void cd_uni_test(){
  for(int i = 0; i < cd_test_length; i++){
    switch(fork()){
      case -1 : return;
      case 0 :
      cd(cd_test[i]);
      execlp("cmds/pwd","pwd",NULL);
      return;
    }
  }
}

int main(int argc, char**argv){
  if(argc < 2) return -1;
  setenv("TARNAME","",1);
  setenv("TARPATH","",1);
  if(strcmp(argv[1],"cd") == 0) cd_uni_test();
}
