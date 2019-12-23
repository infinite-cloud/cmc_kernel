#include <inc/login.h>

#ifndef PATH_H
#define PATH_H

#define NCWD BUFSIZE

extern char *cwd;
extern size_t cwd_len;

int chdir(const char *new_path);
void getcwd(char *path);
int parse_path(char *parsed_path, const char *new_path);

#endif /* !PATH_H */
