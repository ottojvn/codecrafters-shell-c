#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  constexpr int command_size = 1024;
  // Flush after every printf
  setbuf(stdout, NULL);

  printf("$ ");
  char command[command_size];
  fgets(command, command_size, stdin)

      return 0;
}
