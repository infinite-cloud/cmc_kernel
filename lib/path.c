#include <inc/string.h>
#include <inc/path.h>
#include <inc/lib.h>

int
parse_path(char *parsed_path, const char *new_path)
{
	int i, parsed_idx, part_idx;
	char part[BUFSIZE];
	bool stop;

	parsed_idx = 0;
	part_idx = 0;
	stop = false;
	i = 0;
	part[0] = '\0';

	if (new_path[0] != '/')
	{
		getcwd(parsed_path);
		parsed_idx += strnlen(parsed_path, BUFSIZE);

		if (strcmp(parsed_path, "/") || !strcmp(new_path, ".."))
		{
			parsed_path[parsed_idx++] = '/';
		}
	}
	else
	{
		parsed_path[parsed_idx++] = '/';
		i++;
	}

	while (i < BUFSIZE && !stop)
	{
		if (i > 0 && new_path[i - 1] == '/' && new_path[i] == '/')
		{
			return -E_INVAL_PATH;
		}

		if (new_path[i] == '/' || new_path[i] == '\0')
		{
			part[part_idx] = '\0';

			if (new_path[i] == '\0')
			{
				stop = true;
			}

			if (!strcmp(part, ".."))
			{
				parsed_idx--;

				while (parsed_idx > 1 && parsed_path[parsed_idx - 1] != '/')
				{
					parsed_path[parsed_idx--] = '\0';
				}

				part[0] = '\0';
				part_idx = 0;
				i++;
				continue;
			}
			else if (!strcmp(part, "."))
			{
				part[0] = '\0';
				part_idx = 0;
				i++;
				continue;
			}

			strncpy(&parsed_path[parsed_idx], part, part_idx);
			parsed_idx += part_idx;
			part[0] = '\0';
			part_idx = 0;
			i++;
			parsed_path[parsed_idx] = '/';

			if (!stop)
			{
				parsed_idx++;
			}
		}
		else
		{
			part[part_idx++] = new_path[i++];
		}
	}

	while (parsed_idx > 1 && parsed_path[parsed_idx - 1] == '/')
	{
		parsed_idx--;
	}

	parsed_path[parsed_idx] = '\0';

	return 0;
}

int
chdir(const char *new_path)
{
	int r;
	char buf[BUFSIZE];
	struct Stat st;

	if ((r = parse_path(buf, new_path)) < 0)
	{
		return r;
	}

	if ((r = stat(buf, &st)) < 0)
	{
		return r;
	}

	if (!st.st_isdir)
	{
		return -E_NOT_A_DIR;
	}

	sys_chdir(buf, strnlen(buf, BUFSIZE) + 1);

	return 0;
}

void
getcwd(char *path)
{
	sys_getcwd(path);
}
