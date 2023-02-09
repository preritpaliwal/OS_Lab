      //     else
        //     {
        //         // if there is no pipe in the command
        //         if (num_pipe_cmds == 1)
        //         {
        //             child_pid = fork();

        //             if (child_pid == -1)
        //             {
        //                 perror("Failed to fork.\n");
        //                 exit(1);
        //             }

        //             if (child_pid == 0){ // child process
                        
        //                 // printf("CHILD:This is the child process.\n");
        //                 // printf("CHILD:The child process ID is %d.\n", getpid());

        //                 // handle the redirection of input and output
        //                 handle_io_redirect(piped_cmds_seq[0]);

        //                 if (execvp(piped_cmds_seq[0]->args->data[0], piped_cmds_seq[0]->args->data) == -1)
        //                 {
        //                     perror("Failed to execute command.\n");
        //                     exit(1);
        //                 }


                        

        //                 exit(0);
        //             }
        //             else
        //             { // parent process
        //                 if (piped_cmds_seq[0]->background == 0)
        //                 { // the parent process waits for the child process to finish
        //                     ret_pid = waitpid(child_pid, &status, 0);
        //                     if (ret_pid == -1)
        //                     {
        //                         perror("Failed to wait for child process.\n");
        //                         exit(1);
        //                     }
        //                     printf("PARENT: The child process %d has terminated.\n", ret_pid);
        //                 }
        //                 else {
        //                     ret_pid = waitpid(child_pid, &status, WNOHANG);
        //                     if (ret_pid == -1){
        //                         perror("Failed to wait for child process.\n");
        //                         exit(1);
        //                     }
        //                     printf("ret_pid = %d\n", ret_pid);
        //                     if(ret_pid != 0){
        //                         printf("PARENT: The child process %d has terminated.\n", ret_pid);
        //                     }
        //                     // fprintf(log_fp, "PARENT: The child process %d has terminated.\n", ret_pid);
        //                 }

        //             }
        //         }
        //     }
        // }

        // else if (num_pipe_cmds > 1)
        // {
        //     printf("\n\nHandling PIPED.\n\n");

        //     int pipe_in = 0;
        //     int pipe_fd[2];
        //     memset(pipe_fd, 0, sizeof(pipe_fd));

        //     int n = num_pipe_cmds;

        //     for (int i = 0; i < n - 1; i++)
        //     {
        //         pipe(pipe_fd);

        //         child_pid = fork();

        //         if (child_pid == -1)
        //         {
        //             perror("Failed to fork.\n");
        //             exit(1);
        //         }

        //         if (child_pid == 0)
        //         { // child process

        //             // printf("It is the %dth child process in the pipe with PID = %d.\n\n", i + 1, getpid());

        //             // handle the redirection of input and output
        //             // handle_io_redirect(piped_cmds_seq[0]);

        //             close(STDIN_FILENO);
        //             dup2(pipe_in, STDIN_FILENO);

        //             close(STDOUT_FILENO);
        //             dup2(pipe_fd[1], STDOUT_FILENO);

        //             // if (execvp(piped_cmds_seq[0]->args->data[0], piped_cmds_seq[0]->args->data) == -1)
        //             if (execvp(piped_cmds_seq[i]->args->data[0], piped_cmds_seq[i]->args->data) == -1)
        //             {
        //                 perror("Failed to execute command.\n");
        //                 exit(1);
        //             }

        //             exit(0);
        //         }

        //         close(pipe_fd[1]);

        //         pipe_in = pipe_fd[0];
        //     }

        //     pipe(pipe_fd);

        //     child_pid = fork();

        //     if (child_pid == -1)
        //     {
        //         perror("Failed to fork.\n");
        //         exit(1);
        //     }

        //     if (child_pid == 0)
        //     {

        //         // handle_io_redirect(piped_cmds_seq[0]);

        //         close(STDIN_FILENO);
        //         dup2(pipe_in, STDIN_FILENO);
        //         // printf("It is the last [%d] child process in the pipe with PID = %d.\n\n", n, getpid());

        //         if (execvp(piped_cmds_seq[n - 1]->args->data[0], piped_cmds_seq[n - 1]->args->data) == -1)
        //         {
        //             perror("Failed to execute command.\n");
        //             exit(1);
        //         }

        //         exit(0);
        //     }
        // }

// static void reapProcesses(int sig) {
//   pid_t pid;
//   while (true) {
//     pid = waitpid(-1, NULL, WNOHANG);
//     if (pid <= 0) break;
//     if (pid == fgpid) fgpid = 0;
//   }

//   exitUnless(pid == 0 || errno == ECHILD, kWaitFailed, stderr, "waitpid function failed");
// }
