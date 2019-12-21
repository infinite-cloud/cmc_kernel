#include <inc/lib.h>

#define DEFAULT_USER "user"
#define DEFAULT_PASSWORD "password"
#define DEFAULT_HOME "/home/user"
#define DEFAULT_SHELL "/sh"

static bool
validate_member(const char *member)
{
	int i;

	for (i = 0; i < BUFSIZE; i++)
	{
		if (member[i] == '\0')
		{
			break;
		}

		if (member[i] == SEPARATOR)
		{
			return false;
		}
	}

	return true;
}

void
usage(void)
{
	cprintf("Usage: useradd [-d HOME_DIR] [-p PASSWORD] "
		"[-s SHELL] [LOGIN]\n");
	exit();
}

void
umain(int argc, char *argv[])
{
	int r, c, fd_passwd, fd_shadow;
	char record[BUFSIZE * 3];
	const char *home, *password, *shell, *login;
	struct Passwd passwd;
	struct Shadow shadow;
	struct Argstate args;

	home = NULL;
	password = NULL;
	shell = NULL;
	login = NULL;

	argstart(&argc, argv, &args);

	while ((r = argnext(&args)) >= 0)
	{
		switch(r)
		{
			case 'd':
				home = argnextvalue(&args);
				break;
			case 'p':
				password = argnextvalue(&args);
				break;
			case 's':
				shell = argnextvalue(&args);
				break;
			default:
				usage();
				break;
		}
	}

	if (argc > 2)
	{
		usage();
	}
	else if (argc == 2)
	{
		login = argv[1];
	}

	login = (login) ? login : DEFAULT_USER;
	password = (password) ? password : DEFAULT_PASSWORD;
	shell = (shell) ? shell : DEFAULT_SHELL;
	home = (home) ? home : DEFAULT_HOME;

	if (!validate_member(login))
	{
		cprintf("Invalid login\n");
		exit();
	}

	if (!validate_member(shell))
	{
		cprintf("Invalid shell\n");
		exit();
	}

	if (!validate_member(home))
	{
		cprintf("Invalid home\n");
		exit();
	}

	if ((fd_passwd = open("/etc/passwd", O_RDWR)) < 0)
	{
		cprintf("useradd: open: %i\n", fd_passwd);
		exit();
	}

	if ((fd_shadow = open("/etc/shadow", O_RDWR)) < 0)
	{
		cprintf("useradd: open: %i\n", fd_shadow);
		exit();
	}

	if ((r = find_record(fd_passwd, login, record, PASSWD_MEMBERS_NUM)) < 0 ||
		r > 0)
	{
		cprintf("useradd: find_record: %i\n", r);
		exit();
	}

	if ((r = find_record(fd_shadow, login, record, SHADOW_MEMBERS_NUM)) < 0 ||
		r > 0)
	{
		cprintf("useradd: find_record: %i\n", r);
		exit();
	}

	while ((r = read(fd_passwd, &c, sizeof(char))) != 0)
	{
		if (r < 0)
		{
			cprintf("useradd: read: %i\n", r);
			exit();
		}
	}

	c = '\n';

	if ((r = write(fd_passwd, &c, sizeof(char))) < 0)
	{
		cprintf("useradd: write: %i\n", r);
		exit();
	}

	while ((r = read(fd_shadow, &c, sizeof(char))) != 0)
	{
		if (r < 0)
		{
			cprintf("useradd: read: %i", r);
			exit();
		}
	}

	c = '\n';

	if ((r = write(fd_shadow, &c, sizeof(char))) < 0)
	{
		cprintf("useradd: write: %i\n", r);
		exit();
	}

	if ((r = open(home, O_MKDIR | O_CREAT)) < 0)
	{
		cprintf("useradd: open: %i\n", r);
		exit();
	}

	strncpy(passwd.user_name, login, BUFSIZE);
	strncpy(passwd.user_path, home, BUFSIZE);
	strncpy(passwd.user_shell, shell, BUFSIZE);
	parse_from_passwd(record, &passwd);

	if ((r = write(fd_passwd, (const void *) record,
		strnlen(record, BUFSIZE * 3))) < 0)
	{
		cprintf("useradd: write: %i\n", r);
		exit();
	}

	close(fd_passwd);

	strncpy(shadow.user_name, login, BUFSIZE);
	generate_salt(shadow.user_salt);
	crypt(password, shadow.user_salt, shadow.user_hash);
	parse_from_shadow(record, &shadow);

	if ((r = write(fd_shadow, (const void *) record,
		strnlen(record, BUFSIZE * 3))) < 0)
	{
		cprintf("useradd: write: %i\n", r);
		exit();
	}

	close(fd_shadow);
}
