#include <inc/string.h>
#include <inc/path.h>
#include <inc/lib.h>

static char path[BUFSIZE];

void
chdir(const char *new_path)
{
	sys_chdir(new_path, strnlen(new_path, BUFSIZE) + 1);
}

const char *
getcwd(void)
{
	sys_getcwd(path);
	return path;
}
