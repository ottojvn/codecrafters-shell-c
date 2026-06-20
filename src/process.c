#include "process.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static bool in_dir(const char *dir_path, const char *cmd, char *full_path) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        return false;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip current and parent (. and ..)
        if (strcmp(entry->d_name, cmd) != 0) {
            continue;
        }

        if (snprintf(full_path, BUFSIZ, "%s%s%s", dir_path, DIR_DELIMITER, cmd) < 0) {
            perror("snprintf");
            continue;
        }

        // get file information
        struct stat file_stat;
        if (stat(full_path, &file_stat) == 0 &&                            // got file stat
            (S_ISREG(file_stat.st_mode) && file_stat.st_mode & S_IXUSR)) { // is executable
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}

bool in_path(const char *cmd, char *full_path) {
    char *path_env = getenv("PATH");
    char path[BUFSIZ];
    strncpy(path, path_env, BUFSIZ);
    const char *dir_path = NULL;
    dir_path = strtok(path, PATH_LIST_DELIMITER);
    if (dir_path != NULL && (int)(in_dir(dir_path, cmd, full_path))) {
        return true;
    }
    while ((dir_path = strtok(NULL, PATH_LIST_DELIMITER)) != NULL) {
        if (in_dir(dir_path, cmd, full_path)) {
            return true;
        }
    }
    return false;
}

int execute(const char *cmd_path, char *const argv[]) {
    pid_t pid = fork();
    int status;
    switch (pid) {
    case -1:
        perror("fork");
        return (EXIT_FAILURE);
    case 0:
        execvp(cmd_path, argv);
        perror("execvp");
        _exit(127);
    default:
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }

        return 1;
    }
}
