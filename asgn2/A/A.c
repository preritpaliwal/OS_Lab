            // else if (!strcmp(piped_cmds_seq[0]->args->data[0], "jobs")){
            //     if (piped_cmds_seq[0]->args->size > 1){ // more than 1 argument in the jobs command
            //         printf("Invalid command! No args allowed with the jobs command.\n");
            //         continue;
            //     }
            //     else{
            //         // print the jobs
            //     }
            // }

            // else if (!strcmp(piped_cmds_seq[0]->args->data[0], "fg")){
            //     if (piped_cmds_seq[0]->args->size > 2){ // more than 1 argument in the fg command
            //         printf("Invalid command! Only 1 arg (job id) allowed with the fg command.\n");
            //         continue;
            //     }
            //     else{
            //         // bring the job to the foreground
            //     }
            // }

            // else if (!strcmp(piped_cmds_seq[0]->args->data[0], "bg")){
            //     if (piped_cmds_seq[0]->args->size > 2){ // more than 1 argument in the bg command
            //         printf("Invalid command! Only 1 arg
            
#include<stdio.h>
#include<stdlib.h>

int main(){

    int num = 0;
    scanf("%d", &num);

    char *usr_input = malloc(100);
    scanf("%s", usr_input);
    
    printf("%s\n", usr_input);



    for (int i = 0; i <= num; i++){
        printf("Hello World, %d\n", i);
    }
    return 0;
}