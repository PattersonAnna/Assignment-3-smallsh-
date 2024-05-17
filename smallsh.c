#include "smallsh.h"

//when user is ready to exit the shell
void exitProgram(){
    exit(EXIT_SUCCESS);
}

void getCD(char *userInput){
    printf("%s, in getCD\n", userInput);
    if(chdir(userInput) == 0){
        printf("success");
    }
    start();
}

void getStatus(){
    start();
}
/*correctly call calls the functions that handle the built-in commands and the function 
that will hand the other commands*/
void start(){
    char userInput[2049];
    char cwd[2049];

    do{
        printf(":");
        scanf("%2048s", userInput);

        if(strcmp(userInput, "exit") == 0){
            exitProgram();
        }else if(strcmp(userInput, "status") == 0){
            printf("%s", userInput);
        }else if(strcmp(userInput, "pwd") == 0){
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        }else if(userInput[0] == 'c' && userInput[1] == 'd'){
            getCD(userInput);
        }else if(userInput[0] == 'c' && userInput[1] == 'd' && userInput[2] != ' '){
            getCD(userInput);
        }else if(userInput[0] == '#'){
            continue;
        }else if(userInput[0] == '\0'){
            continue;
        }else{
            printf("test2");
            printf("%s", userInput);
        }
    }while(strcmp(userInput, "exit") != 0);
}

int main(int argc, char *argv[]){
    printf("$ smallsh\n");
    start();
    return 0;
}
