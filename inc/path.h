#include <inc/login.h>

#ifndef PATH_H
#define PATH_H

#define NCWD BUFSIZE

/*
 * This is a buffer for the current working directory.
 * It is allocated by the kernel.
 */
extern char *cwd;
/*
 * This variable contains the length of the current working
 * directory. It is calculated by the kernel during the
 * sys_chdir() syscall.
 */
extern size_t cwd_len;

/*
 * This function takes a non-null pointer to the new path
 * (either absolute or relative) and parses it into an absolute
 * path which is written into the buffer pointed to by
 * 'parsed_path'. '..' and '.' sequences are processed in a
 * way similar to UNIX-like systems.
 * The caller is responsible for allocating a buffer of
 * at least BUFSIZE bytes long.
 */
int parse_path(char *parsed_path, const char *new_path);
/*
 * This function takes a non-null pointer to a string 'new_path'
 * containing the new path.
 * Returns:
 * 	negative if an error occurs,
 * 	zero otherwise.
 */
int chdir(const char *new_path);
/*
 * This function writes the current working directory into
 * a buffer pointed to by 'path'.
 * The caller is responsible for * allocating a buffer of at
 * least BUFSIZE bytes long.
 */
void getcwd(char *path);

#endif /* !PATH_H */
