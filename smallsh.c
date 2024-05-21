#include "smallsh.h"

//I know it's not the best to use gobal varibles but this made this easier
char homeDir[2049];

//exit status
int status = 0;

//when user is ready to exit the shell
void exitProgram(){
    exit(EXIT_SUCCESS);
}

int numArgs(struct Command *current){
    int num = 0;
    int i = 0;
    while(current->arg[i] != NULL){
        num++;
        i++;
    }
    return num;
}

void freeCommand(struct Command *commands){
    struct Command *current = commands;
    int args = numArgs(current);
        free(current->command);
        free(current->input);
        free(current->output);
        for(int i = 0; i < args; i++){
            free(current->arg[i]);
        }
        free(current);
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
    start();
}
void getStatus(){
    printf("exit value %d\n", status);
    start();
}

void executingOtherCommands(struct Command *current){
    pid_t newChild = fork();

    if(newChild == 0){
        if(current->arg[1] == NULL && current->input == NULL && current->output == NULL && current->background == false){
        execlp(current->command, current->arg[0], NULL);
        }
    }else{
        //the parent waits for the child to finish running before allowing anyother processes to be run
        if (!current->background) {
            waitpid(newChild, &status, 0);
        }
        freeCommand(current);

    }
}

void slitCommand(char *userInput) {
    // Initialize the struct
    struct Command *current = malloc(sizeof(struct Command));
    int argIndex = 0;

    char *token = strtok(userInput, " ");
    current->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(current->command, token);

    current->arg[argIndex] = calloc(strlen(token) + 1, sizeof(char));
    strcpy(current->arg[argIndex], token);
    argIndex++;

    current->input = NULL;
    current->output = NULL;
    current->background = false;

    // The while loop splits up the user input and creates an array of arguments,
    // sets the input file and output file, and determines if the command should
    // be run in the background with a boolean value
    token = strtok(NULL, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            current->input = calloc(strlen(token) + 1, sizeof(char));
            token = strtok(NULL, " ");
            strcpy(current->input, token);
        } else if (strcmp(token, ">") == 0) {
            current->output = calloc(strlen(token) + 1, sizeof(char));
            token = strtok(NULL, " ");
            strcpy(current->output, token);
        } else if (strcmp(token, "&") == 0) {
            current->background = true;
        } else {
            current->arg[argIndex] = calloc(strlen(token) + 1, sizeof(char));
            strcpy(current->arg[argIndex], token);
            argIndex++; // Increment argIndex for each valid argument
        }
        token = strtok(NULL, " ");
    }
    current->arg[argIndex] = NULL; 
    //printCommand(current);
    executingOtherCommands(current);
}

void printCommand(struct Command *current) {
    printf("Command: %s\n", current->command);
    printf("Arguments:\n");
    for (int i = 1; current->arg[i] != NULL; i++) { // Check the loop condition
        printf("  %s\n", current->arg[i]); // Check the indexing
    }
    if (current->input) {
        printf("Input file: %s\n", current->input);
    }
    if (current->output) {
        printf("Output file: %s\n", current->output);
    }
    printf("Background: %s\n", current->background ? "true" : "false");
}

/*correctly call calls the functions that handle the built-in commands and the function 
that will hand the other commands*/
void start(){
    char userInput[2049];
    char cwd[2049];
    printf("back to start");

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
            slitCommand(userInput);
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
