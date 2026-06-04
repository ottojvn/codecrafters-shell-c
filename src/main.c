#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  printf("$ ");
  char command[BUFSIZ];
  fgets(command, BUFSIZ, stdin);
  strtok(command, "\n");

  printf("%s: command not found\n", command);

  return 0;
}
