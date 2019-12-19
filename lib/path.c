#include <inc/string.h>
#include <inc/path.h>
#include <inc/lib.h>

static char path[BUFSIZE];

int
chdir(const char *new_path)
{
	int r;
	size_t len;
	char buf[BUFSIZE * 2 + 1];
	struct Stat st;

	if (new_path[0] != '/')
	{
		strncpy(buf, getcwd(), BUFSIZE);
		len = strnlen(path, BUFSIZE);
		buf[len] = '/';
		strncpy(buf + len + 1, new_path, BUFSIZE);
	}

	if ((r = stat(buf, &st)) < 0)
	{
		return r;
	}

	if (!st.st_isdir)
	{
		return -1;
	}

	sys_chdir(buf, strnlen(buf, BUFSIZE) + 1);

	return 0;
}

const char *
getcwd(void)
{
	sys_getcwd(path);
	return path;
}
