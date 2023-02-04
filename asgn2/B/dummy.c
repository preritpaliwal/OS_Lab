#include <stdio.h>
#include <time.h>
int main()
{
    char buff[1024];

    for (int i = 0; i < 1000000000; i++)
    {
        if (i % 10000000 == 0)
            printf("%d. \n", i);
        // sleep(1)
    }
}

// ./a.out infile.txt
// ./a.out < infile.txt