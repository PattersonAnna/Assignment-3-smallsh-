#include "smallsh.h"

//I know it's not the best to use gobal varibles but this made this easier
char homeDir[2049];

int currentStatus = 0;

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

void ignoreCtrlC(int signum){
    int saved_errno = errno;
    int status;

    printf("terminated by signal %d\n", WTERMSIG(status));
    errno = saved_errno;
}

void ignore_pkill(int signal){
    signal(SIGTERM, SIG_IGN);
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
    if (WIFEXITED(currentStatus)) {
        printf("exit value %d\n", WEXITSTATUS(currentStatus));
    }
}

void executingOtherCommands(struct Command *current) {
    //printCommand(current);
    pid_t newChild = fork();
    if (newChild == 0) {
        signal(SIGTERM, SIG_IGN);
        // Child process
        if(current->arg[1] == NULL && current->input == NULL && current->output == NULL && current->background == false) {
            if(execlp(current->command, current->arg[0], NULL) == -1){
                printf("%s : no such file or directory\n", current->command);
                exit(1);
            }else{
                exit(0);
            }
        } else if(current->input != NULL && current->output != NULL) {
            int inputFile = open(current->input, O_RDONLY);
            int outputFile = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(inputFile, STDIN_FILENO);
            dup2(outputFile, STDOUT_FILENO);
            close(inputFile);
            close(outputFile);
            execvp(current->command, current->arg);
        } else if(current->input != NULL) {
            if(access(current->input,  F_OK) == 0){
                int inputFile = open(current->input, O_RDONLY);
                dup2(inputFile, STDIN_FILENO);
                close(inputFile);
                execvp(current->command, current->arg);
                exit(0);
            }else{
                printf("cannot open %s for input\n", current->input);
                exit(1);
            }
        } else if(current->output != NULL) {
            if(access(current->output,  F_OK) == 0){
                int outputFile = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(outputFile, STDOUT_FILENO);
                close(outputFile);
                execvp(current->command, current->arg);
                exit(0);
            }else{
                printf("cannot open %s for input\n", current->output);
                exit(1);
            }
        } else if(current->arg[1] != NULL && strcmp(current->arg[1], "-f") != 0) {
            int inputFile = open(current->arg[1], O_RDONLY);
            dup2(inputFile, STDIN_FILENO);
            close(inputFile);
            execvp(current->command, current->arg);
        }else if(strcmp(current->arg[1], "-f") == 0){
            struct stat buffer;
            int state = stat(current->arg[2], &buffer);
            if(state == 0 && S_ISREG(buffer.st_mode)){
                exit(0);
            }else{
                exit(1);
            }
        }else{
            start();
        }
    } else if(newChild > 0) {
        // Parent process
        int exitStatus;
        if (!current->background) {
            waitpid(newChild, &exitStatus, 0);
            currentStatus = exitStatus;
        }
    }
}

void background(struct Command *current){
    pid_t newChild = fork();
    int childID = getpid();

    if(newChild == 0){

        if(current->background == true){
            setsid();
        }

        if(current->arg[1] == NULL && current->input == NULL && current->output == NULL && current->background == false) {
            if(execlp(current->command, current->arg[0], NULL) == -1){
                printf("%s : no such file or directory\n", current->command);
                exit(1);
            }else{
                exit(0);
            }
        } else if(current->input != NULL && current->output != NULL) {
            int inputFile = open(current->input, O_RDONLY);
            int outputFile = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(inputFile, STDIN_FILENO);
            dup2(outputFile, STDOUT_FILENO);
            close(inputFile);
            close(outputFile);
            execvp(current->command, current->arg);
        } else if(current->input != NULL) {
            if(access(current->input,  F_OK) == 0){
                int inputFile = open(current->input, O_RDONLY);
                dup2(inputFile, STDIN_FILENO);
                close(inputFile);
                execvp(current->command, current->arg);
                exit(0);
            }else{
                printf("cannot open %s for input\n", current->input);
                exit(1);
            }
        } else if(current->output != NULL) {
            if(access(current->output,  F_OK) == 0){
                int outputFile = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(outputFile, STDOUT_FILENO);
                close(outputFile);
                execvp(current->command, current->arg);
                exit(0);
            }else{
                printf("cannot open %s for input\n", current->output);
                exit(1);
            }
        } else if(current->arg[1] != NULL && strcmp(current->arg[1], "-f") != 0) {
            int inputFile = open(current->arg[1], O_RDONLY);
            dup2(inputFile, STDIN_FILENO);
            close(inputFile);
            execvp(current->command, current->arg);
        }else if(strcmp(current->arg[1], "-f") == 0){
            struct stat buffer;
            int state = stat(current->arg[2], &buffer);
            if(state == 0 && S_ISREG(buffer.st_mode)){
                exit(0);
            }else{
                exit(1);
            }
        }else{
            start();
        }
        
    }else if (newChild > 0) {
        printf("background pid is %d\n", newChild);
    }
}

void handle_sigchld(int sig){
    int saved_errno = errno;
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("background pid %d is done: exit value %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("background pid %d is done: terminated by signal %d\n", pid, WTERMSIG(status));
        }
    }
    errno = saved_errno;
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
    if(current->background == true){
        checkBackground(current);
    }else{
       executingOtherCommands(current);
    }
    freeCommand(current);
}

void checkBackground(struct Command *current){
    char cwd[2049];

    if(strcmp(current->command, "exit") == 0){
        exitProgram();
    }
     else if(strcmp(current->command, "status") == 0 ){
        getStatus();
    }
    else if(strcmp(current->command, "pwd") == 0){
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
    }
    else if(strcmp(current->command, "cd") == 0){
        getCD(current->command );
    }
    else if((strcmp(current->command, "#") == 0)){
        start();
    }
    else if((strcmp(current->command, "\0") == 0)){
        start();
    }else{
        background(current);
    }
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
        if(strcmp(userInput, "status") == 0 ){
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
    signal(SIGINT, ignoreCtrlC);
    signal(SIGCHLD, ignore_pkill);
    signal(SIGTERM, ignore_pkill);
    struct passwd *pw = getpwuid(getuid());                 //gets the home directory of the current user, pw built for c
    strncpy(homeDir, pw->pw_dir, sizeof(homeDir) - 1);      //uses the pw struct to acces the path to the home directory
    
    // Set up SIGCHLD handler
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    start();
    return 0;
}



