#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "parse.h"
#include "io_redirect.h"

#define MAX_BUFF_SIZE 1024

cmd **tokenise_on_pipe(char *user_input, char *err, int *err_flag, int *num_cmds)
{
    vector_string *piped_cmds = vector_string_init(MAX_ARGS);
    char *temp = strdup("");

    for (int i = 0; i < strlen(user_input) - 1; i++)
    {
        if (user_input[i] == '"')
        {
            str_concat_char(temp, user_input[i]);
            i++;
            while (i < strlen(user_input) - 1 && (user_input[i] != '"' || (user_input[i] == '"' && user_input[i - 1] == '\\')))
            {
                str_concat_char(temp, user_input[i]);
                i++;
            }
            if (i == strlen(user_input) - 1)
            {
                strcpy(err, "Invalid command! Error in quotes");
                *err_flag = -1;
                return NULL;
            }
            if (user_input[i] == '"')
            {
                str_concat_char(temp, user_input[i]);
            }
        }

        else if (user_input[i] == '|')
        {
            if (i == strlen(user_input) - 2)
            { // the user has entered a command like "ls | wc |"
                strcpy(err, "Invalid command! No command after a pipe.\n");
                *err_flag = -1;
                return NULL;
            }

            if (strlen(temp) != 0)
            {
                vector_string_push_back(piped_cmds, temp);
                // printf("%s\n", temp);
                temp = strdup("");

                while (user_input[i + 1] == ' ' && i < strlen(user_input) - 1)
                { // remove the unnnecesarry spaces after pipe
                    i++;
                }
                if (i == strlen(user_input))
                { // the user has entered a command like "ls | wc |   "
                    strcpy(err, "Invalid command! No command after a pipe.\n");
                    *err_flag = -1;
                    return NULL;
                }
            }

            else
            { // the user has entered a command like "ls || wc" or the cmd starts with a pipe
                strcpy(err, "Invalid command! Error in using pipe.\n");
                *err_flag = -1;
                return NULL;
            }
        }

        else
        {
            str_concat_char(temp, user_input[i]);
        }
    }

    if (strlen(temp) != 0)
    {
        vector_string_push_back(piped_cmds, temp);
    }

    cmd **piped_cmds_seq = (cmd **)malloc(piped_cmds->size * sizeof(cmd *));
    int bg_flag = 0;

    for (int i = 0; i < piped_cmds->size; i++)
    {
        printf("cmd %d: %s\n", i, piped_cmds->data[i]);
    }

    // check if the & is present in the last command in the seq of piped commands
    for (int i = 0; i < piped_cmds->size; i++)
    {
        // if the & is not present in the last command in the seq of piped commands
        // printf("%s\n", piped_cmds->data[i]);
        if (strchr(piped_cmds->data[i], '&') != NULL)
        {
            // printf("Hi!\n");
            if (i != piped_cmds->size - 1)
            {
                strcpy(err, "Invalid command! Cannot have an & in a cmd if it is not the last cmd in a seq of piped cmds.\n");
                *err_flag = -1;
                return NULL;
                // printf("Hello!\n");
            }
            else
            {
                bg_flag = 1;
            }
        }
    }

    *num_cmds = piped_cmds->size;

    for (int i = 0; i < piped_cmds->size; i++)
    {
        piped_cmds_seq[i] = cmd_init(piped_cmds->data[i]);
        if (bg_flag == 1 && i == piped_cmds->size - 1)
        {
            piped_cmds_seq[i]->background = 1;
        }
    }

    // *err_flag = 0;
    return piped_cmds_seq;
}

int main()
{

    pid_t child_pid, ret_pid;
    int status;
    //  exit_flag = 0;
    // char *args[] =  {"ls", "-l", "-R", "-a", NULL};

    while (1)
    {
        printf("\033[1;31m"); // set the text color to red
        printf("%s:", getenv("USER"));
        printf("\033[1;33m");            // set the text color to yellow
        printf("%s$ ", getcwd(NULL, 0)); // getcwd() returns the current working directory (char *
        printf("\033[0m");

        size_t input_size = 0;
        char *user_input = NULL;

        // read the user input
        if (getline(&user_input, &input_size, stdin) == -1)
        {
            free(user_input);
            perror("Failed to read input.\n");
            exit(1);
        }

        // tokenize the user input on the basis of the pipe character
        char *err = strdup("");
        int err_flag = 0;
        int num_pipe_cmds = 0;
        cmd **piped_cmds_seq = tokenise_on_pipe(user_input, err, &err_flag, &num_pipe_cmds);

        // if the user has entered an invalid command
        if (err_flag == -1)
        {
            if (piped_cmds_seq != NULL)
            {
                for (int i = 0; i < num_pipe_cmds; i++)
                {
                    cmd_free(piped_cmds_seq[i]);
                }
                free(piped_cmds_seq);
            }
            printf("%s\n", err);
            continue;
        }
        err = strdup("");

        for (int i = 0; i < num_pipe_cmds; i++)
        {

            // tokenise the individual commands
            if (cmd_parse(piped_cmds_seq[i], err) == -1)
            {
                printf("%s\n", err);
                err_flag = -1;
                break;
            }
        }

        if (err_flag == -1)
        { // atleast one of the subcommands is invalid
            continue;
        }

        for (int i = 0; i < num_pipe_cmds; i++)
        {
            cmd_print(piped_cmds_seq[i]);
            printf("\n\n");
        }

        // if the user has entered a valid command and there is no pipe in the command
        if (num_pipe_cmds == 1)
        {

            if (!strcmp(piped_cmds_seq[0]->args->data[0], "exit"))
            {

                if (piped_cmds_seq[0]->args->size > 2)
                { // more than 1 argument in the exit command
                    printf("Invalid command! Only 1 numeric arg (exit status) allowed with the exit command.\n");
                    continue;
                }
                else
                {

                    if (piped_cmds_seq[0]->args->size == 2)
                    { // the user has entered an exit status
                        int exit_status = atoi(piped_cmds_seq[0]->args->data[1]);
                        printf("Exiting the shell with exit status %d.\n", exit_status);
                        exit(exit_status);
                    }
                    else
                    { // the user has not entered an exit status
                        printf("Exiting the shell.\n");
                        exit(0);
                    }
                }
            }

            else if (!strcmp(piped_cmds_seq[0]->args->data[0], "cd"))
            {
                if (piped_cmds_seq[0]->args->size > 2)
                { // more than 1 argument in the cd command
                    printf("cd: Too many arguments.\n");
                    continue;
                }
                else
                {
                    if (piped_cmds_seq[0]->args->size == 2)
                    { // the user has not entered a path
                        if (!strcmp(piped_cmds_seq[0]->args->data[1], "~"))
                        { // the user has entered ~ as the path
                            chdir(getenv("HOME"));
                        }
                        else if (!strcmp(piped_cmds_seq[0]->args->data[1], "-"))
                        { // the user has entered - as the path
                            chdir(getenv("OLDPWD"));
                        }
                        else
                        { // the user has entered a path
                            if (chdir(piped_cmds_seq[0]->args->data[1]) == -1)
                            {
                                perror("cd: ");
                            }
                        }
                    }

                    else if (piped_cmds_seq[0]->args->size == 1)
                    { // the user has not entered a path
                        chdir(getenv("HOME"));
                    }
                }
            }

            else
            {
                // if there is no pipe in the command
                if (num_pipe_cmds == 1)
                {
                    child_pid = fork();

                    if (child_pid == -1)
                    {
                        perror("Failed to fork.\n");
                        exit(1);
                    }

                    if (child_pid == 0)
                    { // child process

                        printf("CHILD:This is the child process.\n");
                        printf("CHILD:The child process ID is %d.\n", getpid());

                        // handle the redirection of input and output
                        handle_io_redirect(piped_cmds_seq[0]);

                        if (execvp(piped_cmds_seq[0]->args->data[0], piped_cmds_seq[0]->args->data) == -1)
                        {
                            perror("Failed to execute command.\n");
                            exit(1);
                        }

                        exit(0);
                    }
                    else
                    { // parent process
                        if (piped_cmds_seq[0]->background == 0)
                        { // the parent process waits for the child process to finish
                            ret_pid = waitpid(child_pid, &status, 0);
                            if (ret_pid == -1)
                            {
                                perror("Failed to wait for child process.\n");
                                exit(1);
                            }
                            printf("PARENT: The child process %d has terminated.\n", ret_pid);
                        }

                        else
                        {
                            ret_pid = waitpid(-1, &status, WNOHANG);
                            if (ret_pid == -1){
                                perror("Failed to wait for the child process.\n");
                                exit(1);
                            }
                            printf("PARENT: The child process %d has terminated.\n", ret_pid);

                        }
                        
                    }
                }
            }
        }

        // the 'exit' command cannot be run in the child process as it will terminate the child process
        // not the parent (shell) process. So we check if the command is 'exit' and if so,
        // we do not fork a child process, instead we set the exit_flag to 1 and break out of the loop
        // and exit the shell program.
        //     if(strcmp(user_input, "exit") == 0){
        //         exit_flag = 1;
        //         break;
        //     }

        //     if (tokenize_cmd(user_input, args, &num_tokens) == -1){ // case when too many args and flags in cmd
        //         continue;
        //     }
        //     else for (int i = 0; i < num_tokens; i++){
        //         printf("args[%d] = %s\n", i, args[i]);
        //     }

        //     if (strcmp(args[0], "cd") == 0){
        //         if (num_tokens == 1){
        //             chdir(getenv("HOME"));
        //         }
        //         else if (num_tokens == 2){
        //             if(strcmp(args[1], "~") == 0){
        //                 chdir(getenv("HOME"));
        //             }
        //             else if (strcmp(args[1], "-") == 0){
        //                 chdir(getenv("OLDPWD"));
        //             }
        //             // remove the quotes from the string
        //             else if (args[1][0] == '\''){
        //                 char *temp = strtok(args[1], "\'");
        //                 args[1] = (char *)malloc(strlen(temp) + 1);
        //                 strcpy(args[1], temp);
        //                 if (chdir(args[1]) == -1){
        //                     printf("%s\n", args[1]);
        //                     perror("chdir");
        //                 }

        //             }
        //             // remove the double quotes from the string
        //             else if (args[1][0] == '\"'){
        //                 char *temp = strtok(args[1], "\"");
        //                 args[1] = (char *)malloc(strlen(temp) + 1);
        //                 strcpy(args[1], temp);
        //                 if (chdir(args[1]) == -1){
        //                     printf("%s\n", args[1]);
        //                     perror("chdir");
        //                 }
        //             }
        //             else if (chdir(args[1]) == -1){
        //                 printf("%s\n", args[1]);
        //                 perror("chdir");
        //             }

        //         }
        //         else{
        //             printf("Too many arguments.\n");
        //         }
        //     }

        //     else{
        //         child_pid = fork();

        //         if (child_pid == -1){
        //             perror("fork");
        //             exit(1);
        //         }

        //         if(child_pid == 0){
        //             printf("CHILD:This is the child process.\n");
        //             printf("CHILD:The child process ID is %d.\n", getpid());

        //             if (execvp(args[0], args) == -1){
        //                 perror("execvp");
        //                 exit(1);
        //             }
        //         }
        //         else{
        //             ret_pid = wait(&status);
        //             printf("PARENT: The child process ID is %d.\n", child_pid);
        //             printf("PARENT: The child process %d has terminated.\n", ret_pid);
        //         }
        //     }
        // }

        // if (exit_flag == 1){
        //     printf("Exiting the shell...\n");
        //     exit(0);
        // }
    }
    return 0;
}