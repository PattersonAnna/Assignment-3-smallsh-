#include "smallsh.h"

//when user is ready to exit the shell
void exitProgram(){
    exit(EXIT_SUCCESS);
}

// void getCD(char *userInput){
//     char cwd[2049];
//     getcwd(cwd, sizeof(cwd));
//     char *home = strtok(cwd, "/");
//     char *token = strtok(userInput, " ");
//     token = strtok(NULL, " ");

//     if(token  == NULL){
//         chdir(home);
//     }else{
//         chdir(token);
//     }


//     printf("%s, in getCD\n", userInput);
//     if(chdir(userInput) == 0){
//         printf("success");
//     }
//     start();
// }

void getCD(char *userInput) {
    char cwd[2049];
    char *token = strtok(userInput, " ");
    token = strtok(NULL, " ");
    char *home = getcwd(cwd, sizeof(cwd));

    if (token == NULL) {
        for(int i = 1; i < 4; i++){
            printf("%d", i);
            home = strtok(home, "/");
        }
        printf("%s", home);
        (chdir(home) == 0);
        
    }
    (chdir(token) == 0); 
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
        fgets(userInput, sizeof(userInput), stdin);

        // Remove trailing newline character if present
        size_t len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\n') {
            userInput[len - 1] = '\0';
        }

        if(strcmp(userInput, "exit") == 0){
            exitProgram();
        }
        if(strcmp(userInput, "status") == 0){
            printf("%s", userInput);
        }
        if(strcmp(userInput, "pwd") == 0){
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        }
        if(userInput[0] == 'c' && userInput[1] == 'd'){
            getCD(userInput);
        }
        if(userInput[0] == '#'){
            continue;
        }
        if(userInput[0] == '\0'){
            continue;
        }
        if(strcmp(userInput, "exit") != 0 && strcmp(userInput, "status") != 0 && strcmp(userInput, "pwd") != 0 && (userInput[0] != '#' && userInput[0] == '\0')){
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
