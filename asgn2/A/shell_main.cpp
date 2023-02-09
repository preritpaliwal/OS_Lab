#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "parse.h"
#include <set>
#include <vector>
#include <iostream>
#include <signal.h>

#include "utils.h"
#include "io_redirect.h"

#define MAX_BUFF_SIZE 1024

using namespace std;

int curr_pg_id = 0;
set <int> fg_processes_pid;
set <int> bg_processes_pid;

int fg_process_group_id = 0;

// fpid is the pid of the foreground process if there is no pipe, else it is the pid of the last process in the pipe
static pid_t fgpid = 0; // 0 means no foreground process 

static void manage_child(int sig){
    int status;
    pid_t ret_pid;
    while(true){
        ret_pid = waitpid(-1, &status, WNOHANG);
        if (ret_pid == 0){  // there are one or more child processes in execution, but none of them have exited 
            break;
        }

        else if (ret_pid == -1){  // there are no child processes in execution, or there was an error
            // perror("waitpid");
            break;
        }

        else{
            if (ret_pid == fgpid){ // if the child process that exited was the foreground process
                fgpid = 0;  // set the fgpid to 0 to denote that no foreground process is running from now on
            }
            
            if (fg_processes_pid.find(ret_pid) != fg_processes_pid.end()){  // if the process that exited was a foreground process
                fg_processes_pid.erase(ret_pid);  // remove the pid of the foreground process from the set of foreground processes
                
                if (WIFEXITED(status)){
                    printf("\nFg process with pid %d exited normally with status %d\n", ret_pid, WEXITSTATUS(status));
                }
                else{
                    printf("\nFg process with pid %d exited abnormally with status %d\n", ret_pid, WEXITSTATUS(status));    
                }
            
            }

            if (bg_processes_pid.find(ret_pid) != bg_processes_pid.end()){  // if the process that exited was a background process
                bg_processes_pid.erase(ret_pid);  // remove the pid of the background process from the set of background processes
            
                if(WIFEXITED(status)){
                    printf("\nBg process with pid %d exited normally with status %d\n", ret_pid, WEXITSTATUS(status));
                }
                else{
                    printf("\nBg process with pid %d exited abnormally with status %d\n", ret_pid, WEXITSTATUS(status));    
                }
            }

        }

    }
    return;
}

void sigint_handler(int sig){
    // signal(SIGINT, sigint_handler);
    // printf("\n");
    // fflush(stdout);
    return;
}

static void wait_for_fg_process(pid_t pid) {
    fgpid = pid;
    sigset_t empty; // empty set of signals
    sigemptyset(&empty);   // initialize the empty set of signals 
    while (fgpid == pid) {

        // When some unblocked signal arrives, the process gets the CPU, the signal is handled, the original blocked
        // set is restored, and sigsuspend returns.
        sigsuspend(&empty);   
    }
    unblock_SIGCHLD();
}

void force_bg_run_handler(int sig){
    // signal(SIGTSTP, force_bg_run_handler);

    if (fgpid != 0){ // if there is a foreground process running
    
    // // send the process to background and let it execute in the background using SIGTSTP followed by SIGCONT
    // kill(-1*fg_process_group_id, SIGTSTP);  // send the foreground process group a SIGTSTP signal to stop the process
    // kill(-1*fg_process_group_id, SIGCONT); // now the process is in the background, send a SIGCONT signal to continue the process
        fgpid = 0;  // set the fgpid to 0 to denote that no foreground process is running from now on
    }
    // return;
}


void handle_process(cmd ** cmd_seq, int *num_piped_cmds, int background){
    
    curr_pg_id = 0;
    fg_process_group_id = 0;
    //clear the set of foreground processes, if any
    fg_processes_pid.clear();

    int pipefd[2];
    int last_cmd_pid = 0;
    memset(pipefd, 0, sizeof(pipefd));
    printf("No of cmds: %d\n", *num_piped_cmds);

    for (int i = 0; i < *num_piped_cmds; i++){

        printf("%d command in process:\n", i);

        cmd_seq[i]->in_fd = STDIN_FILENO;  // set the input file descriptor of a command to STDIN_FILENO
        cmd_seq[i]->out_fd = STDOUT_FILENO; // set the output file descriptor of a command to STDOUT_FILENO
        
        handle_io_redirect(cmd_seq[i]);  // opens the i/o redirection files, if any and sets the file descriptors in the cmd struct
        if (cmd_seq[i]->in_redirect == 1)
            printf("Hi\n");

        if (i > 0){  // for all other commands except 1st one in pipe , set the input file descriptor to the read end of the pipe
                     // for 1st command, its input file descriptor is already handled by the handle_io_redirect function
            cmd_seq[i]->in_fd = pipefd[0];  
        }

        if (i < *num_piped_cmds -1 ){

            if (pipe(pipefd) == -1){
                perror("pipe: ");
                exit(1);
            }


            cmd_seq[i]->out_fd = pipefd[1];  // set the output file descriptor of a command to the write end of the pipe
        }

        block_SIGCHLD();  // block the SIGCHLD signal, so that the child process does not get killed before the parent process has a chance to add it to the set of foreground processes
        
        pid_t child_pid = fork();

        if (child_pid == -1){
            perror("Failed to fork.\n");
            exit(1);
        }

        if (i == *num_piped_cmds - 1){  // if the command is the last command in the pipe
            last_cmd_pid = child_pid;  // set the last_cmd_pid to the pid of the last command in the pipe
        }

        if (child_pid == 0){

            unblock_SIGCHLD();  // unblock the SIGCHLD signal in the child process as it may also fork its own child processes and we want to be able to handle the SIGCHLD signal in the child process

            // for i == 0, set the process group id of the 1st child process to its own pid 
            // set the process group id of the 1st child process to its own pid 
            // (i.e. the 1st child process in a piped set of cmds 
            // becomes the leader of its own process group, and 
            // the other child processes in the piped set of cmds 
            // become the members of the process group of the 1st child process)
            
            // for i > 0, set the process group id of the child process to the process group id of the 1st child process

            if (setpgid(0, curr_pg_id) == -1){
                perror("setpgid: ");
                exit(1);
            }


            if (cmd_seq[i]->in_fd != STDIN_FILENO){  // if the input file descriptor of the command is not STDIN_FILENO, then close the STDIN_FILENO file descriptor
                // close(STDIN_FILENO);
                dup2(cmd_seq[i]->in_fd, STDIN_FILENO);  // duplicate the input file descriptor of the command to the STDIN_FILENO file descriptor
                close(cmd_seq[i]->in_fd);
            }

            if (cmd_seq[i]->out_fd != STDOUT_FILENO){  // if the output file descriptor of the command is not STDOUT_FILENO, then close the STDOUT_FILENO file descriptor
                // close(STDOUT_FILENO);
                dup2(cmd_seq[i]->out_fd, STDOUT_FILENO);  // duplicate the output file descriptor of the command to the STDOUT_FILENO file descriptor
                close(cmd_seq[i]->out_fd);
            }

            // printf("Executing command: %s\n", cmd_seq[i]->args->data[0]);
            if (execvp(cmd_seq[i]->args->data[0], cmd_seq[i]->args->data) == -1){
                perror("Failed to execute command.\n");
                exit(1);
            }
        }

        // setpgid also  called in the parent as well as the child process as we don't know which process will execute first
        setpgid(child_pid, curr_pg_id);  // set the process group id of the child process to the process group id of the 1st child process 
        if (curr_pg_id == 0){

            // curr_pg_id is given the value of the pid of the 1st child process 
            // so that the other child processes in the piped set of cmds 
            // can be added to the process group of the 1st child process 
            curr_pg_id = child_pid;  
            fg_process_group_id = child_pid;  // set the process group id of the foreground process group to the process group id of the 1st child process

            if (background == 0){  // for the process grp with curr_pg_id (this process is assumed to start in fg), set it as the foreground process grp on the terminal associated with the stdin file descriptor
                tcsetpgrp(STDIN_FILENO, curr_pg_id); // this will block a background process from accessing STDIN of terminal until the foreground process finishes
            }

        }


        if (!background)  // if the command is not a background process, add it to the set of foreground processes
            fg_processes_pid.insert(child_pid);
        
        else
            bg_processes_pid.insert(child_pid);
        
        unblock_SIGCHLD();  // unblock the SIGCHILD signal now, so that the parent process can handle the SIGCHLD signal

        if (i < *num_piped_cmds -1 ){  // close the read end of the pipe, if it is not the last command in the pipe
            close(cmd_seq[i]->out_fd);
        }

    }

    if (background == 1){  // if the command is a background process, print the pid of the last command in the pipe
        unblock_SIGCHLD();
    }

    else if (background == 0){  // if the command is not a background process, wait for the last command in the pipe to finish executing
        wait_for_fg_process(last_cmd_pid);
    

        // reset the process grp of the parent(our own shell) as the foreground process grp on the terminal associated with the stdin file descriptor
        if (tcsetpgrp(STDIN_FILENO, getpgid(0)) == -1){
            perror("tcsetpgrp: ");
            exit(1);
        }
        
        // // clear the cmd seq if the foreground process has finished executing
        // for (int i = 0; i < *num_piped_cmds; i++){
        //     cmd_free(cmd_seq[i]);
        // };

    }


    // clear the set of foreground processes, if any
    fg_processes_pid.clear();

    curr_pg_id = 0;  // reset the curr_pg_id to 0
    fg_process_group_id = 0;  // reset the fg_process_group_id to 0
}


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
    printf("Num commands: %d\n", *num_cmds);

    for (int i = 0; i < piped_cmds->size; i++)
    {
        piped_cmds_seq[i] = cmd_init(piped_cmds->data[i]);
        if (bg_flag == 1 && i == piped_cmds->size - 1)
        {
            piped_cmds_seq[i]->background = 1;
        }
    }
    printf("No error\n");

    // *err_flag = 0;
    return piped_cmds_seq;
}


int main()
{

    FILE * log_fp;
    log_fp = fopen("log.txt", "a+");

    pid_t child_pid, ret_pid;
    // int status;

    fg_processes_pid.clear();
    bg_processes_pid.clear();


    // setting the signal handlers     
    signal(SIGCHLD, manage_child); // SIGCHLD is sent to the parent process when a child process terminates either normally or abnormally

    signal(SIGINT, sigint_handler); // SIGINT is sent to the process when the user presses Ctrl+C

    signal(SIGTSTP, force_bg_run_handler); // SIGTSTP is sent to the process when the user presses Ctrl+Z, but here we need to send process to the background if Ctrl+Z is pressed

    signal(SIGTTOU, SIG_IGN); 

    while(1){
        printf("\033[1;31m");  // set the text color to red
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
        int background_flag = 0;
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
            if (piped_cmds_seq[i]->background == 1){
                background_flag = 1;
            }
        }

        if (err_flag == -1)
        { // atleast one of the subcommands is invalid
            continue;
        }

        for (int i = 0; i < num_pipe_cmds; i++)
        {
            printf("cmd no: %d\n", i);
            cmd_print(piped_cmds_seq[i]);
            printf("\n\n");
        }


        if (num_pipe_cmds == 0){
            continue;
        }

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

        else {
            printf("This is the parent process.\n");
            handle_process(piped_cmds_seq, &num_pipe_cmds, background_flag);
        }
 
    }
    fclose(log_fp);
    return 0;
}


