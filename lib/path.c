#include <inc/string.h>
#include <inc/path.h>

char path[BUFSIZE];

void
set_path(const char *new_path)
{
	strncpy(path, new_path, BUFSIZE);
}

const char *
get_path(void)
{
	return path;
}
