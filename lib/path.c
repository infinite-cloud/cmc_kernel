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

	/* If new_path is not an absolute path, get an absolute path.
	   Add a slash to the end regardless. */
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
		/* We already considered a slash from the absolute path */
		i++;
	}

	while (i < BUFSIZE && !stop)
	{
		/* If a slash follows a slash ('//'), we have encountered
		   an incorrect path. Return an error */
		if (i > 0 && new_path[i - 1] == '/' && new_path[i] == '/')
		{
			return -E_INVAL_PATH;
		}

		/* A part of the path is limited by either two slashes,
		   or a slash and an EOL */
		if (new_path[i] == '/' || new_path[i] == '\0')
		{
			/* Null-terminate, so that we can compare
			   it to '.' and '..' */
			part[part_idx] = '\0';

			/* If we encounter an EOL, exit
			   in the end of current iteration*/
			if (new_path[i] == '\0')
			{
				stop = true;
			}

			if (!strcmp(part, ".."))
			{
				/* Wipe the previous part.
				   However if all we have is '/',
				   do nothing*/
				parsed_idx--;

				while (parsed_idx > 1 &&
					parsed_path[parsed_idx - 1] != '/')
				{
					parsed_path[parsed_idx--] = '\0';
				}

				/* Empty the buffer */
				part[0] = '\0';
				part_idx = 0;
				i++;
				continue;
			}
			else if (!strcmp(part, "."))
			{
				/* Do nothing, simply empty the buffer */
				part[0] = '\0';
				part_idx = 0;
				i++;
				continue;
			}

			/* Add a part to the path and terminate with a slash,
			   then empty the buffer */
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
			/* Simply copy this symbol if the part is not over */
			part[part_idx++] = new_path[i++];
		}
	}

	/* There is still a slash in the end from the last iteration.
	   Remove it if this isn't a root directory */
	while (parsed_idx > 1 && parsed_path[parsed_idx - 1] == '/')
	{
		parsed_idx--;
	}

	/* Null-terminate and return */
	parsed_path[parsed_idx] = '\0';

	return 0;
}

int
chdir(const char *new_path)
{
	int r;
	char buf[BUFSIZE];
	struct Stat st;

	/* Parse the path into an absolute path or return an error
	   if this isn't possible */
	if ((r = parse_path(buf, new_path)) < 0)
	{
		return r;
	}

	/* We do that in order to check whether this is
	   a folder or not */
	if ((r = stat(buf, &st)) < 0)
	{
		return r;
	}

	if (!st.st_isdir)
	{
		return -E_NOT_A_DIR;
	}

	/* Do a syscall. We pass strnlen() + 1 because that one byte is
	   needed for a null-terminator*/
	sys_chdir(buf, strnlen(buf, BUFSIZE) + 1);

	return 0;
}

void
getcwd(char *path)
{
	/* Do a syscall */
	sys_getcwd(path);
}
