#include "pwd.h"

int main(void){
    char* pwd = getpwd();
    write(STDIN_FILENO, pwd, strlen(pwd));
    free(pwd);
    return 0;
}

char* getpwd(){
  char* pwd = getcwd(NULL,0);
  if(strlen(getenv("TARNAME")) > 0){
    pwd = realloc(pwd,strlen(pwd) + strlen(getenv("TARNAME")) + 2);
    strcat(pwd,"/");
    strcat(pwd,getenv("TARNAME"));
  }
  if(strlen(getenv("TARPATH")) > 0){
    pwd = realloc(pwd,strlen(pwd) + strlen(getenv("TARPATH")) + 2);
    strcat(pwd,"/");
    strcat(pwd,getenv("TARPATH"));
  }
  pwd = realloc(pwd,strlen(pwd) + 2);
  strcat(pwd,"\n");
  strcat(pwd,"\0");
  return pwd;
}
