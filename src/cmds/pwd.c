#include "pwd.h"

int main(void){
    char* pwd = getpwd();
    write(STDIN_FILENO, pwd, strlen(pwd));
    return 0;
}

char* getpwd(){
  char* pwd = getcwd(NULL,0);
  char* pwd_plus = getenv("TARPATH");
  if(pwd_plus == NULL) return "";
  char* total_pwd = malloc(sizeof(char) * (strlen(pwd) + strlen(pwd_plus)) + 2);
  memset(total_pwd, '\0',(strlen(pwd) + strlen(pwd_plus)) + 2);
  strcat(total_pwd,pwd);
  strcat(total_pwd,pwd_plus);
  strcat(total_pwd,"\n");

  return total_pwd;
}
