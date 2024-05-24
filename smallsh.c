#include "smallsh.h"

//I know it's not the best to use gobal varibles but this made this easier
char homeDir[2049];

//global variable to keep track of the status
int currentStatus = 0;

int terminatedStatus = 0;

int backgroundStatus = 0;

//when user is ready to exit the shell
void exitProgram(){
    exit(EXIT_SUCCESS);
}

//used to get the number of args for the freeing the correct amount of memory
int numArgs(struct Command *current){
    int num = 0;
    int i = 0;
    while(current->arg[i] != NULL){
        num++;
        i++;
    }
    return num;
}

//allows for ^c to be used on a child process and prints the correct message with status num
void childIgnoreCtrlC(int signum){
    terminatedStatus = SIGINT;
    printf("terminated by signal %d\n", SIGINT);  //WTERMSIG used to get the signal number
}

//for parent and background child
void handleSigint(int signum){
    printf("\nCtrl+C ignored in the main process.\n");
    //does nothing for the parent and the background child
}

void customCtrlz(int signum){
    if(backgroundStatus == 0){
        backgroundStatus = 1;
        printf("Entering foreground-only mode (& is now ignored)\n");
    }else if(backgroundStatus == 1){
        backgroundStatus = 0;
        printf("Exiting foreground-only mode\n");
    }
    
}

//frees all of the memory allocated during the spliting of the command input
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

//used for cd check if path it given, if not return to home directory, ignores &
void getCD(char *userInput) {
    char *token = strtok(userInput, " ");
    token = strtok(NULL, " ");  //this will get the name of the directory the user want to move to
   
    // No directory specified, change to home directory else change to the given path
    if (token == NULL) {
        chdir(homeDir); //chdir is a bulit in function
    }else if(strcmp(token , "&") == 0){
        chdir(homeDir);
    }else{
        chdir(token);
    }
    start();
}

//checks the status if commands are succesful they return 0, if not then they return 1
void getStatus(){
    if(terminatedStatus != 0){
        printf("terminated by signal %d\n", terminatedStatus);  //WTERMSIG used to get the signal number
    }else{
        if (WIFEXITED(currentStatus)) {     //WIFEXITED checks if the child exited normaly
            printf("exit value %d\n", WEXITSTATUS(currentStatus));  //WEXITSTATUS gets the exit status of the child process
        }
    }
}

//used for pirnting the message when the background process has finished running
void checkBackgroundStatus(){
    int exitStatus;
    pid_t pid = waitpid(-1, &exitStatus, WNOHANG);  //waitpid waits for any child process to finish, &status stores the status and, WNOHANG prevents it from waiting for a child t finish
    while (pid > 0) {
        if (WIFEXITED(exitStatus)) {
            printf("background pid %d is done: exit value %d\n", pid, WEXITSTATUS(exitStatus)); //WEXITSTATUS gets the exit status of the child process
        } else if (WIFSIGNALED(exitStatus)) {
            printf("background pid %d is done: terminated by signal %d\n", pid, WTERMSIG(exitStatus));  //WTERMSIG used to get the signal number
        }
        pid = waitpid(-1, &exitStatus, WNOHANG);    //this stores the process id of the child that just finished running
    }

}

//used to run forground commands
void executingOtherCommands(struct Command *current) {
    pid_t newChild = fork();
    signal(SIGINT, childIgnoreCtrlC);    //custom signal handler so that ^c works ona child process
    signal(SIGTSTP, SIG_IGN);
    if (newChild == 0) {    // if a child process was succesfully created should be 0
        if(current->arg[1] == NULL && current->input == NULL && current->output == NULL && current->background == false) { 
            if(execlp(current->command, current->arg[0], NULL) == -1){  //if a command runs successfully, it returns 0
                printf("%s : no such file or directory\n", current->command);   //command was unsuccessful so an error is printed and exit status is set to 1
                fflush(stdout);
                exit(1);
            }else{
                exit(0);
            }
        } else if(current->input != NULL && current->output != NULL) {  //if an input and out file are given, open both
            int inputFile = open(current->input, O_RDONLY); //read only
            int outputFile = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(inputFile, STDIN_FILENO);  //dup2 is used so standard input will read from the given file
            dup2(outputFile, STDOUT_FILENO); // this dup2 is used so standard out will write to the given file
            close(inputFile);
            close(outputFile);
            execvp(current->command, current->arg); //preforms the commands as a child process
        } else if(current->input != NULL) {
            if(access(current->input,  F_OK) == 0){ //checks that the file exists if not then an error is printed
                int inputFile = open(current->input, O_RDONLY);
                dup2(inputFile, STDIN_FILENO);
                close(inputFile);
                execvp(current->command, current->arg);
                exit(0);
            }else{
                printf("cannot open %s for input\n", current->input);
                fflush(stdout);
                exit(1);
            }
        } else if(current->output != NULL) {
            int outputFile = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, 0644); //write only, create if it doesn't exist, create file of contence if already open
            dup2(outputFile, STDOUT_FILENO);
            close(outputFile);
            execvp(current->command, current->arg);
            exit(0);
        } else if(current->arg[1] != NULL && strcmp(current->arg[1], "-f") != 0) {
            int inputFile = open(current->arg[1], O_RDONLY);
            dup2(inputFile, STDIN_FILENO);
            close(inputFile);
            execvp(current->command, current->arg);
        }else if(strcmp(current->arg[1], "-f") == 0){
            struct stat buffer; //holds info about the file
            int state = stat(current->arg[2], &buffer); //gets information about the given file
            if(state == 0 && S_ISREG(buffer.st_mode)){  //checks if stat was successful and if it's a regular file
                exit(0);
            }else{
                exit(1);
            }
        }else{
            start();
        }
    } else if(newChild > 0) {   //checks the child process has finished
        // Parent process
        int exitStatus;
        if (!current->background) {
            waitpid(newChild, &exitStatus, 0);  //
            currentStatus = exitStatus;
        }
    }
}


//same as executingOtherCommands() but for background use
void background(struct Command *current){
    pid_t newChild = fork();
    signal(SIGINT, SIG_IGN); // Ignore SIGINT
    signal(SIGTSTP, SIG_IGN);   //ignore SIGTSTP
    int childID = getpid();

    if(newChild == 0){
        if(current->arg[1] == NULL && current->input == NULL && current->output == NULL && current->background == true) {
            int input = open("/dev/null", O_RDONLY);
            int output = open("/dev/null", O_WRONLY);
            dup2(input, STDIN_FILENO);
            dup2(output, STDOUT_FILENO);
            close(input);
            close(output);

            if(execlp(current->command, current->arg[0], NULL) == -1){
                printf("%s : no such file or directory\n", current->command);
                fflush(stdout);
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
                fflush(stdout);
                exit(1);
            }
        } else if(current->output != NULL) {
            int outputFile = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(outputFile, STDOUT_FILENO);
            close(outputFile);
            execvp(current->command, current->arg);
            exit(0);
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
        fflush(stdout);
    }
}

//function ignore & if it comes after a built in command
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

/*take in the command, allocates mem for the whole struct, then adds args till reaches a symbol
if a > the follow string is stored as the output filename, < does the same thing, & sets the backgroud value true*/
void slitCommand(char *userInput) {
    // Initialize the struct
    struct Command *current = malloc(sizeof(struct Command));   //allocates mem for the whole struct
    int argIndex = 0;


    char *token = strtok(userInput, " ");   //takes the first argument
    current->command = calloc(strlen(token) + 1, sizeof(char)); //allocated mem for command
    strcpy(current->command, token);    //copes the token into command 


    current->arg[argIndex] = calloc(strlen(token) + 1, sizeof(char));   //sets the command as the first argument
    strcpy(current->arg[argIndex], token);
    argIndex++;

    current->input = NULL;
    current->output = NULL;
    current->background = false;

    // The while loop splits up the user input and creates an array of arguments,
    // sets the input file and output file, and determines if the command should
    // be run in the background with a boolean value
    token = strtok(NULL, " ");  //moves token to the next argument in the user input
    while (token != NULL) {     //loops through until all of the elements in the user input have been assigned to the correct struct element 
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            current->input = calloc(strlen(token) + 1, sizeof(char));
            strcpy(current->input, token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            current->output = calloc(strlen(token) + 1, sizeof(char));
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
    if(current->background == true && backgroundStatus == 0){    //if user enter & run the command in the background
        checkBackground(current);
    }else{
       executingOtherCommands(current);
    }
    freeCommand(current);
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

void checkExpansion(char *userInput) {
    char result[2049]; // Buffer to hold the result
    char *ptr; // Pointer for strstr function result
    char pidStr[15]; // String to store process ID

    // Get the process ID and convert it to string
    pid_t pid = getpid();
    snprintf(pidStr, sizeof(pidStr), "%d", pid);

    // Copy userInput to result
    strcpy(result, userInput);

    // Search for "$$" in result
    while ((ptr = strstr(result, "$$")) != NULL) {
        // Replace "$$" with process ID in result
        strcpy(ptr, pidStr);
        // Shift result to remove extra characters
        memmove(ptr + strlen(pidStr), ptr + 2, strlen(ptr + 2) + 1);
    }

    // Copy the modified result back to userInput
    strcpy(userInput, result);
}


/*correctly call calls the functions that handle the built-in commands and the function
that will hand the other commands*/
void start(){
    char userInput[2049];
    char cwd[2049];
    char temp[2049];

    do{
        checkBackgroundStatus();
        printf(":");
        fgets(userInput, sizeof(userInput), stdin);
        //signal(SIGINT, handleSigintBackgroundChild);

        checkExpansion(userInput);
        printf("%s", userInput);
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
            getcwd(cwd, sizeof(cwd));                           //uses the struct passwd inilized in the main function
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
        if(strncmp(userInput, "pkill", 6) == 0) {
            //executeCommand(userInput);
        }
        if(strcmp(userInput, "exit") != 0 && strcmp(userInput, "status") != 0 && strcmp(userInput, "pwd") != 0 && (userInput[0] != '#' && userInput[0] != '\0')){
            slitCommand(userInput);
        }
    }while(strcmp(userInput, "exit") != 0);
}

int main(int argc, char *argv[]){
    signal(SIGINT, handleSigint);
    signal(SIGTSTP, customCtrlz); 
    printf("$ smallsh\n");

    struct passwd *pw = getpwuid(getuid());                 //gets the home directory of the current user, pw built for c
    strncpy(homeDir, pw->pw_dir, sizeof(homeDir) - 1);      //uses the pw struct to acces the path to the home directory

    start();
    return 0;
}



