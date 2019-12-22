#include <inc/lib.h>

char buf[BUFSIZE];

int
read_line(int fd, char *record)
{
	int i, r, c;

	for (i = 0; i < BUFSIZE * 3; i++)
	{
		if ((r = read(fd, &c, sizeof(char))) < 0)
		{
			break;
		}

		if (c == '\n' || c == '\r' || r == 0)
		{
			record[i] = '\0';
			break;
		}

		record[i] = c;
	}

	return r;
}

const char *
get_name(const char *record)
{
	int i;

	for (i = 0; i < BUFSIZE + 1; i++)
	{
		if (record[i] == SEPARATOR)
		{
			buf[i] = '\0';
			break;
		}

		buf[i] = record[i];
	}

	return buf;
}

int
copy_file(int fd_from, int fd_to, const char *name, bool rm_dir)
{
	int r, c, eof;
	char record[BUFSIZE * 3];
	struct Passwd passwd;

	do
	{
		r = read_line(fd_from, record);
		eof = r;

		if (r >= 0 && strncmp(name, get_name(record), BUFSIZE))
		{
			if ((r = write(fd_to, record,
				strnlen(record, BUFSIZE * 3))) < 0)
			{
				break;
			}

			c = '\n';

			if ((r = write(fd_to, &c, sizeof(char))) < 0)
			{
				break;
			}
		}
		else if (r >= 0 && rm_dir)
		{
			parse_into_passwd(record, &passwd);

			if ((r = remove(passwd.user_path)) < 0)
			{
				break;
			}
		}

		if (!eof)
		{
			r = eof;
			break;
		}
	} while (r > 0);

	return r;
}

void
usage(void)
{
	cprintf("Usage: userdel [-d] LOGIN\n");
	exit();
}

void
umain(int argc, char *argv[])
{
	int i, r, fd_tmp, fd;
	bool rm_dir;
	const char *files[] =
	{
		"/etc/passwd",
		"/etc/shadow",
	};
	struct Argstate args;

	rm_dir = false;
	argstart(&argc, argv, &args);

	while ((r = argnext(&args)) >= 0)
	{
		switch(r)
		{
			case 'd':
				rm_dir = true;
				break;
			default:
				usage();
				break;
		}
	}

	if (argc != 2)
	{
		usage();
	}

	for (i = 0; i < sizeof(files) / sizeof(files[0]); i++)
	{
		if ((fd_tmp = open("/tmp/userdel.tmp",
			O_WRONLY | O_CREAT | O_TRUNC)) < 0)
		{
			cprintf("userdel: open: %i\n", fd_tmp);
			exit();
		}

		if ((fd = open(files[i], O_RDONLY)) < 0)
		{
			cprintf("userdel: open: %i\n", fd);
			exit();
		}

		if ((r = copy_file(fd, fd_tmp, argv[1], rm_dir)) < 0)
		{
			cprintf("userdel: copy_file: %i\n", r);
			exit();
		}

		close(fd);
		close(fd_tmp);

		if ((fd_tmp = open("/tmp/userdel.tmp", O_RDONLY)) < 0)
		{
			cprintf("userdel: open: %i\n", fd_tmp);
			exit();
		}

		if ((fd = open(files[i], O_WRONLY | O_TRUNC)) < 0)
		{
			cprintf("userdel: open: %i\n", fd);
			exit();
		}

		if ((r = copy_file(fd_tmp, fd, "", false)) < 0)
		{
			cprintf("userdel: copy_file: %i\n", r);
			exit();
		}

		close(fd);
		close(fd_tmp);
	}

	if ((r = remove("/tmp/userdel.tmp")) < 0)
	{
		cprintf("userdel: remove: %i\n", r);
		exit();
	}
}
