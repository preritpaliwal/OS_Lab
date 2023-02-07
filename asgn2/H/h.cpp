#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

void handle_arrow_keys(int key)
{
  switch (key) {
    case 65:
      printf("Up arrow pressed\n");
      break;
    case 66:
      printf("Down arrow pressed\n");
      break;
    case 67:
      printf("Right arrow pressed\n");
      break;
    case 68:
      printf("Left arrow pressed\n");
      break;
    default:
      printf("%c", key);
  }
  return;
}

int main(void)
{
  char *line;
  rl_bind_key('\033', handle_arrow_keys);

  while ((line = readline(">>> ")) != NULL) {
    if (*line) {
      add_history(line);
    }
  }

  return 0;
}
