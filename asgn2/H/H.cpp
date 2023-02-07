// #include <unistd.h>
// #include <termios.h>
// #include <stdio.h>

// char getch() {
//         char buf = 0;
//         struct termios old = {0};
//         if (tcgetattr(0, &old) < 0)
//             perror("tcsetattr()");
//         old.c_lflag &= ~ICANON;
//         old.c_lflag &= ~ECHO;
//         old.c_cc[VMIN] = 1;
//         old.c_cc[VTIME] = 0;
//         if (tcsetattr(0, TCSANOW, &old) < 0)
//             perror("tcsetattr ICANON");
//         if (read(0, &buf, 1) < 0)
//             perror ("read()");
//         old.c_lflag |= ICANON;
//         old.c_lflag |= ECHO;
//         if (tcsetattr(0, TCSADRAIN, &old) < 0)
//             perror ("tcsetattr ~ICANON");
//         return (buf);
// }

// int main() {
//     char c;
//     while (1) {
//         c = getch();
//         if (c == 'q') {
//             break;
//         }
//         printf("%c\n", c);
//     }
//     return 0;
// }


#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main()
{


/* Curses Initialisations */
    initscr();
    
    keypad(stdscr, TRUE);
    cbreak();

    noecho();
    int ch;

    printw("%s:", getenv("USER"));
    printw("%s$ ", getcwd(NULL, 0));  // getcwd() returns the current working directory (char *

    while((ch = getch()) != '\n')
    {
        // printw("\033[1;31m");  // set the text color to red
        printw("%s:", getenv("USER"));
        // printw("\033[1;33m");  // set the text color to yellow
        printw("%s$ ", getcwd(NULL, 0));  // getcwd() returns the current working directory (char *
        // printw("\033[0m");

    while(1)
    {
        // clear();
        // echo();

        // noecho();
        if (ch == '\n'){
            printw("\n");
            break;
        }

        else if(ch == KEY_UP)
            printw("up");
        else if(ch == KEY_DOWN)
            printw("down");
        else if(ch == KEY_LEFT)
            printw("left");
        else if(ch == KEY_RIGHT)
            printw("right");
        else
            printw("%c", ch);

        refresh();
    }

    }



refresh();
// getch();
endwin();

return 0;
}