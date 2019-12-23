#include <inc/lib.h>

/* This buffer is used locally to simplify some interfaces */
char buf[BUFSIZE];

/*
 * Read a record into 'record'
 * from the file with a file descriptor 'fd'
 */
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

/*
 * Extract a name from the record read
 * from either /etc/passwd or /etc/shadow
 */
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

/*
 * Copy a file from 'fd_from' to 'fd_to', except for the record
 * containing a name 'name'. Remove the home directory of
 * the user with a name 'name' if 'rm_dir' is true
 */
int
copy_file(int fd_from, int fd_to, const char *name, bool rm_dir)
{
	int r, c, eof;
	char record[BUFSIZE * 3];
	struct Passwd passwd;

	do
	{
		r = read_line(fd_from, record);
		/* Exit in the end of this iteration if the file ends */
		eof = r;

		/* If no errors occured and this is not a record of the user
		   with a name 'name', copy this record */
		if (r >= 0 && strncmp(name, get_name(record), BUFSIZE))
		{
			if ((r = write(fd_to, record,
				strnlen(record, BUFSIZE * 3))) < 0)
			{
				break;
			}

			/* Put a newline in the end so that the records can
			   be distinguished from each other */
			c = '\n';

			if ((r = write(fd_to, &c, sizeof(char))) < 0)
			{
				break;
			}
		}
		/* This is a record of the user with a name 'name',
		   don't copy it and delete their home directory
		   if necessary */
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

	/* Read args */
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

	/* Must have only two args upon reading '-d'
	   (or not reading it at all)*/
	if (argc != 2)
	{
		usage();
	}

	for (i = 0; i < sizeof(files) / sizeof(files[0]); i++)
	{
		/* Try to create a temporary file */
		if ((fd_tmp = open("/tmp/userdel.tmp",
			O_WRONLY | O_CREAT | O_TRUNC)) < 0)
		{
			cprintf("userdel: open: %i\n", fd_tmp);
			exit();
		}

		/* Open the file we are copying from */
		if ((fd = open(files[i], O_RDONLY)) < 0)
		{
			cprintf("userdel: open: %i\n", fd);
			exit();
		}

		/* Copy into the temporary file */
		if ((r = copy_file(fd, fd_tmp, argv[1], rm_dir)) < 0)
		{
			cprintf("userdel: copy_file: %i\n", r);
			exit();
		}

		rm_dir = false;

		/* Close everything */
		close(fd);
		close(fd_tmp);

		/* Now do it the other way around... */
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

		/* ...but ignore the line we are trying to delete */
		if ((r = copy_file(fd_tmp, fd, "", false)) < 0)
		{
			cprintf("userdel: copy_file: %i\n", r);
			exit();
		}

		close(fd);
		close(fd_tmp);
	}

	/* Remove the temporary file */
	if ((r = remove("/tmp/userdel.tmp")) < 0)
	{
		cprintf("userdel: remove: %i\n", r);
		exit();
	}
}
