#include "parse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glob.h>

cmd *cmd_init(const char *user_input){
    cmd *c = (cmd *)malloc(sizeof(cmd));
    c->full_cmd = (char *)malloc(sizeof(char) * (strlen(user_input) + 1));
    strcpy(c->full_cmd, user_input);
    c->args = vector_string_init(MAX_ARGS);
    c->input_file = NULL;
    c->output_file = NULL;
    c->in_redirect = 0;
    c->out_redirect = 0;
    c->in_fd = 0;
    c->out_fd = 1;
    // c->fd[0] = 0;
    // c->fd[1] = 0;
    c->background = 0;

    return c;
}

void cmd_free(cmd *c){
    vector_string_free(c->args);
    free(c->full_cmd);
    free(c);
}

void str_concat_str(char *s1, char *s2){
    // printf("str_concat_str: realloc \n");
    s1 = (char *)realloc(s1, sizeof(char) * (strlen(s1) + strlen(s2) + 1));
    strcat(s1, s2);
    s1[strlen(s1) + strlen(s2)] = '\0';
}

void str_concat_char(char *s1, char c){
    // printf("str_concat_char: realloc \n");
    s1 = (char *)realloc(s1, sizeof(char) * (strlen(s1) + 2));
    s1[strlen(s1)] = c;
    s1[strlen(s1) + 1] = '\0';
}

int cmd_parse(cmd *c, char *err){
    vector_string *tokens = vector_string_init(MAX_ARGS);
    char *temp = strdup("");
    printf("%d", c->full_cmd[strlen(c->full_cmd) - 1]);
    for (int i = 0; i < strlen(c->full_cmd); i++){

        if (c->full_cmd[i] == '\\'){

            i++;
            if (i < strlen(c->full_cmd) ){
                str_concat_char(temp, c->full_cmd[i]);
                i++;
            }
            else{
                strcpy(err, "Invalid command! Command cannot end with a backslash\n");
                return -1;
            }
        }

        if (c->full_cmd[i] == '<' || c->full_cmd[i] == '>' || c->full_cmd[i] == '&'){

            if (strlen(temp) != 0){
                vector_string_push_back(tokens, temp);
            }
            temp = strdup("");
            str_concat_char(temp, c->full_cmd[i]);
            vector_string_push_back(tokens, temp);
            temp = strdup("");

        }

        else if (c->full_cmd[i] == '"'){

            if (i!= 0 && c->full_cmd[i-1] == '/'){
                str_concat_char(temp, c->full_cmd[i]);
                // printf("%s\n", temp);
                i++;
                while(i < strlen(c->full_cmd) && (c->full_cmd[i] != '"' || (c->full_cmd[i] == '"' && c->full_cmd[i-1] == '\\'))){
                    str_concat_char(temp, c->full_cmd[i]);
                    i++;
                }
                if (i == strlen(c->full_cmd) ){
                    strcpy(err, "Invalid command! Error in handling quotes\n");
                    return -1;
                }
                if (c->full_cmd[i] == '"')
                    str_concat_char(temp, c->full_cmd[i]);
                
            }

            else{
                i++;
                while(i < strlen(c->full_cmd)  && (c->full_cmd[i] != '"' || (c->full_cmd[i] == '"' && c->full_cmd[i-1] == '\\'))){
                    str_concat_char(temp, c->full_cmd[i]);
                    i++;
                }
                if (i == strlen(c->full_cmd)){
                    strcpy(err, "Invalid command! Error in handling quotes\n");
                    return -1;
                }
            }
        }

        else if (c->full_cmd[i] == ' '){
            if (strlen(temp) != 0){
                vector_string_push_back(tokens, temp);
                temp = strdup("");
            }
            else 
                continue;
        }

        else{
            str_concat_char(temp, c->full_cmd[i]);
        }
    }

    if (strlen(temp) != 0){
        vector_string_push_back(tokens, temp);
        // printf("%s\n", temp);
        temp = strdup("");
    }

    for (int i = 0; i < tokens->size; i++){
        // null terminated string
        str_concat_char(tokens->data[i], '\0');
        printf("%s1\n", tokens->data[i]);

    }

    for (int j = 0; j < tokens->size; j++){
        if (!strcmp(tokens->data[j], "<")){
            if (j == tokens->size - 1 || !strcmp(tokens->data[j+1], ">") || !strcmp(tokens->data[j+1], "<")){
                strcpy(err, "Invalid command! Incorrect use of input redirection\n");
                return -1;
            }
            c->input_file = strdup(tokens->data[j+1]);
            c->in_redirect = 1;
            j+=1;
        }

        else if (!strcmp(tokens->data[j], ">")){
            // printf("j: %d\n", j);
            if (j == tokens->size - 1 || !strcmp(tokens->data[j+1], ">") || !strcmp(tokens->data[j+1], "<")){
                strcpy(err, "Invalid command! Incorrect use of input redirection\n");
                return -1;
            }
            c->output_file = strdup(tokens->data[j+1]);
            c->out_redirect = 1;
            j+=1;
        }

        else if (!strcmp(tokens->data[j], "&")){
            if (j != tokens->size - 1){
                strcpy(err, "Invalid command! No other arguments allowed after &\n");
                return -1;
            }
            c->background = 1;
        }

        else{
            // if jth token contains * or ? then it is a wildcard
            if ( (strchr(tokens->data[j], '*') != NULL ) || (strchr(tokens->data[j], '?') != NULL) )
            {
                // invoke the glob function on this token

                // null terminate the token
                // tokens->data[j][strlen(tokens->data[j])] = '\0';
                
                glob_t wildcard_matches;
                memset(&wildcard_matches, 0, sizeof(wildcard_matches));
                printf("globbing %s\n", tokens->data[j]);
               
                int ret_val =  glob(tokens->data[j], GLOB_TILDE | GLOB_BRACE, NULL, &wildcard_matches);

                if (ret_val != 0){   // in case of error in globbing, handle different error cases

                    switch (ret_val){
                        case GLOB_NOSPACE:
                            strcpy(err, "glob failed: insufficient memory\n");
                            break;
                        
                        case GLOB_ABORTED:
                            strcpy(err, "glob failed: read error\n");
                            break;
                        
                        case GLOB_NOMATCH:
                            strcpy(err, "glob failed: no match found\n");
                            break;

                        default:
                            strcpy(err, "glob failed: unknown error\n");
                            break;
                    }

                    globfree(&wildcard_matches);
                    return -1;
                }

                else {
                    printf("globbing successful\n");
                    printf("number of glob matches: %ld\n", wildcard_matches.gl_pathc);
                    for (size_t k = 0; k < wildcard_matches.gl_pathc; k++){
                        // printf("%s\n", wildcard_matches.gl_pathv[i]);
                        vector_string_push_back(c->args, wildcard_matches.gl_pathv[k]);
                    }

                    globfree(&wildcard_matches);
                }
                

            }

            else {
                vector_string_push_back(c->args, tokens->data[j]);
            }
        }
    }

    return 0;

}


void cmd_print(cmd *c){
    printf("full_cmd: %s\n", c->full_cmd);
    printf("args:\n");
    for (int i = 0; i < c->args->size; i++){
        printf("arg[%d]: %s\n", i, c->args->data[i]);
    }
    printf("\n");
    printf("input_file: %s\n", c->input_file);
    printf("output_file: %s\n", c->output_file);
    printf("in_redirect: %d\n", c->in_redirect);
    printf("out_redirect: %d\n", c->out_redirect);
    printf("background: %d\n", c->background);
    return;
}

// int main(){
//     char *err = malloc(sizeof(char) * 100);
//     size_t input_size = 0;
//     char *user_input = NULL;
//     printf("Enter command: ");

//     // read the user input
//     if (getline(&user_input, &input_size, stdin) == -1){
//         free(user_input);
//         perror("Failed to read input.\n");
//         exit(1);
//     }


//     cmd *c = cmd_init(user_input);
//     int res = cmd_parse(c, err);
//     if (res == -1){
//         printf("%s\n", err);
//         return -1;
//     }
//     cmd_print(c);
//     return 0;
// }
    