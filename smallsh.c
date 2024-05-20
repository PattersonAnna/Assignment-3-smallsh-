#include "smallsh.h"

//I know it's not the best to use gobal varibles but this made this easier
char homeDir[2049];

//exit status
int status = 0;

//when user is ready to exit the shell
void exitProgram(){
    exit(EXIT_SUCCESS);
}

void getCD(char *userInput) {
    char *token = strtok(userInput, " ");
    token = strtok(NULL, " ");                  //this will get the name of the directory the user want to move to 
    
    // No directory specified, change to home directory else change to the given path
    if (token == NULL) {
        chdir(homeDir);
    }else{
        chdir(token);
    }
}

void getStatus(){
    printf("exit value %d\n", status);
    start();
}

void executingOtherCommands(char *userInput){
    pid_t newChild = fork();

    if(newChild == -1){
        start();
    }else if(newChild == 0){
        if(strcmp(userInput, "ls") == 0){
            execlp("ls", "ls", NULL);
        }
    }else{
        //the parent waits for the child to finish running before allowing anyother processes to be run
        waitpid(newChild, &status, 0);
    }
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
            getStatus();
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
        if(strcmp(userInput, "exit") != 0 && strcmp(userInput, "status") != 0 && strcmp(userInput, "pwd") != 0 && (userInput[0] != '#' && userInput[0] != '\0')){
            executingOtherCommands(userInput);
        }
    }while(strcmp(userInput, "exit") != 0);
}

int main(int argc, char *argv[]){
    printf("$ smallsh\n");
    struct passwd *pw = getpwuid(getuid());                 //gets the home directory of the current user, pw built for c 
    strncpy(homeDir, pw->pw_dir, sizeof(homeDir) - 1);      //uses the pw struct to acces the path to the home directory

    start();
    return 0;
}
