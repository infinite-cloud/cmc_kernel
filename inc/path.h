#include <inc/login.h>

#ifndef PATH_H
#define PATH_H

#define NCWD BUFSIZE

extern char *cwd;
extern size_t cwd_len;

int chdir(const char *new_path);
const char *getcwd(void);

#endif /* !PATH_H */
