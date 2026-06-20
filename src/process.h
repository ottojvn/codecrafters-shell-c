#ifndef PROCESS_H_
#define PROCESS_H_

#ifdef _WIN32
#define PATH_LIST_DELIMITER ";"
#define DIR_DELIMITER "\\"
#else
#define PATH_LIST_DELIMITER ":"
#define DIR_DELIMITER "/"
#endif // _WIN_32

int execute(const char *cmd_path, char *const argv[]);

bool in_path(const char *cmd, char *full_path);

#endif // PROCESS_H_
