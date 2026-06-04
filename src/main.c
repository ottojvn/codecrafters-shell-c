#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int handle_command(const char *command);

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (true) {
    printf("$ ");
    char command[BUFSIZ];
    fgets(command, BUFSIZ, stdin);
    strtok(command, "\n");
    int ret;
    if ((ret = handle_command(command)) == -1) {
      printf("%s: command not found\n", command);
    } else if (ret == 1) {
      break;
    }
  }

  return 0;
}

int handle_command(const char *command) {
  if (strcmp(command, "exit") == 0) {
    return 1;
  }
  return -1;
}
