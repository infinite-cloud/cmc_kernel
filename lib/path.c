#include <inc/string.h>
#include <inc/path.h>
#include <inc/lib.h>

static char path[BUFSIZE];
static char buf[BUFSIZE * 2 + 1];

static const char *
get_last_dir(const char *full_path)
{
	int i, slash;

	slash = -1;

	for (i = 0; i < BUFSIZE; i++)
	{
		if (full_path[i] == '\0')
		{
			break;
		}
		else if (full_path[i] == '/')
		{
			slash = i;
		}
	}

	strncpy(buf, full_path + slash + 1, BUFSIZE);

	return buf;
}

static void
remove_last_dir(char *full_path)
{
	int i;

	for (i = strnlen(full_path, BUFSIZE) - 1; i >= 0; i--)
	{
		if (full_path[i] == '/')
		{
			full_path[i] = '\0';
			break;
		}

		full_path[i] = '\0';
	}

	full_path[0] = '/';
}

int
chdir(const char *new_path)
{
	int r;
	size_t len;
	const char *dir;
	struct Stat st;

	if (new_path[0] != '/')
	{
		dir = getcwd();
		strncpy(buf, dir, BUFSIZE);

		if (!strncmp(new_path, "..", BUFSIZE))
		{
			remove_last_dir(buf);
		}
		else if (strncmp(new_path, ".", BUFSIZE))
		{
			len = strnlen(path, BUFSIZE);
			buf[len] = '/';
			strncpy(buf + len + 1, new_path, BUFSIZE);
		}
	}
	else
	{
		dir = get_last_dir(new_path);
		strncpy(buf, new_path, BUFSIZE);

		if (!strncmp(dir, ".", BUFSIZE))
		{
			remove_last_dir(buf);
		}
		else if (!strncmp(dir, "..", BUFSIZE))
		{
			remove_last_dir(buf);
			remove_last_dir(buf);
		}
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
