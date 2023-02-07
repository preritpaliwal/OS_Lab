#include<stdio.h>

int main(){
    // for (int i = 0; i < 100000000; i++){
    //     printf("Hello: %d\n", i);
    // }
    // return 0;
    int n;
    scanf("%d", &n);

    char *s;
    scanf("%s", s);
    printf("%s\n", s);

    for (int i = 0; i < n; i++){
        printf("Hello: %d\n", i + 1);
    }
    return 0;

}