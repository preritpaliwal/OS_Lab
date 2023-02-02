#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_BUFF_SIZE 1024
#define MAX_ARGS 128   // max no of arguments and flags allowed with a command = 128

int tokenize_cmd(char *cmd, char **args, int *num_tokens){
    int i = 0; 
    args[0] = strtok(cmd, " ");
    while(args[i] != NULL){
        i++;
        if (i >= MAX_ARGS){
            printf("Too many arguments.\n");
            return -1;
        }
        args[i] = strtok(NULL, " ");
        // if first char of args[i] is "'", then it is a string and we need to tokenize it
        // until we find the closing "'"
        // if (args[i][0] == '\''){
        //     char *temp = strtok(NULL, "\'");
        //     args[i] = (char *)malloc(strlen(args[i]) + strlen(temp) + 2);
        //     args[i][strlen(args[i])] = ' ';
        //     strcpy(args[i] +)
        //     strcpy(args[i] + strlen(args[i]), temp);
        //     args[i][strlen(args[i]) + strlen(temp) + 1] = "\'";
        //     // strcat(args[i], temp);
        // }
    }
    *num_tokens = i;
    return 0;
}

int main(){
    
    char *user_input = (char *)malloc(MAX_BUFF_SIZE);
    char **args = (char **)malloc((MAX_ARGS+1)*sizeof(char *));
    for (int i = 0; i < MAX_ARGS; i++){
        args[i] = NULL;
    }
    int num_tokens = 0;
    pid_t child_pid, ret_pid;
    int status, exit_flag = 0;
    // char *args[] =  {"ls", "-l", "-R", "-a", NULL};

    while(1){
        printf("\033[1;31m");  // set the text color to red
        printf("%s:", getenv("USER"));
        printf("\033[1;33m");  // set the text color to yellow
        printf("%s$ ", getcwd(NULL, 0));  // getcwd() returns the current working directory (char *
        printf("\033[0m");

        fgets(user_input, MAX_BUFF_SIZE, stdin);
        user_input[strlen(user_input)-1] = '\0';

        // the 'exit' command cannot be run in the child process as it will terminate the child process
        // not the parent (shell) process. So we check if the command is 'exit' and if so, 
        // we do not fork a child process, instead we set the exit_flag to 1 and break out of the loop
        // and exit the shell program.
        if(strcmp(user_input, "exit") == 0){
            exit_flag = 1;
            break;
        }

        if (tokenize_cmd(user_input, args, &num_tokens) == -1){ // case when too many args and flags in cmd
            continue;
        }
        else for (int i = 0; i < num_tokens; i++){
            printf("args[%d] = %s\n", i, args[i]);
        }

        if (strcmp(args[0], "cd") == 0){
            if (num_tokens == 1){
                chdir(getenv("HOME"));
            }
            else if (num_tokens == 2){
                if(strcmp(args[1], "~") == 0){
                    chdir(getenv("HOME"));
                }
                else if (strcmp(args[1], "-") == 0){
                    chdir(getenv("OLDPWD"));
                }
                else if (chdir(args[1]) == -1){
                    perror("chdir");
                }
            }
            else{
                printf("Too many arguments.\n");
            }
        }

        else{ 
            child_pid = fork();


            if (child_pid == -1){
                perror("fork");
                exit(1);
            }

            if(child_pid == 0){
                printf("CHILD:This is the child process.\n");
                printf("CHILD:The child process ID is %d.\n", getpid());

                if (execvp(args[0], args) == -1){
                    perror("execvp");
                    exit(1);
                }
            }
            else{
                ret_pid = wait(&status);
                printf("PARENT: The child process ID is %d.\n", child_pid); 
                printf("PARENT: The child process %d has terminated.\n", ret_pid);
            }
        }
    }

    if (exit_flag == 1){
        printf("Exiting the shell...\n");
        exit(0);
    }

    return 0;
}