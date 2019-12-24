#include <inc/lib.h>
#include <inc/time.h>

/* The screen size. It is used by clear_scr() */
#define CRT_ROWS 25
#define CRT_COLS 80

/*
 * Clears the screen
 */
void
clear_scr(void)
{
	int i;

	/* Skip all rows */
	for (i = 0; i < CRT_ROWS; i++)
	{
		cputchar('\n');
	}

	/* Go back to the beginning of a terminal screen */
	for (i = 0; i < CRT_ROWS * CRT_COLS; i++)
	{
		cputchar('\b');
	}
}

/*
 * Checks whether the login credentials are valid and spawns the user-chosen
 * shell if they are. Clears screen before entering a shell session
 * and upon exiting it if 'clear' is set.
 */
int
auth(const char *login, const char *password, bool clear)
{
	int fd, r, r0;
	char passwd_record[BUFSIZE * PASSWD_MEMBERS_NUM];
	char shadow_record[BUFSIZE * SHADOW_MEMBERS_NUM];
	char buf[BUFSIZE];
	struct Passwd passwd;
	struct Shadow shadow;

	/* Try to get the proper records from /etc/passwd and /etc/shadow,
	   return if any errors occur */
	if ((fd = open("/etc/passwd", O_RDONLY)) < 0)
	{
		return fd;
	}

	if ((r0 = find_record(fd, login, passwd_record,
		PASSWD_MEMBERS_NUM)) < 0)
	{
		return r0;
	}

	close(fd);

	if ((fd = open("/etc/shadow", O_RDONLY)) < 0)
	{
		return fd;
	}


	if ((r = find_record(fd, login, shadow_record,
		SHADOW_MEMBERS_NUM)) < 0)
	{
		return r;
	}

	close(fd);

	/* A record being present in one file but not in the other
	   means that the files are not coherent, something must be broken */
	if ((r == 0 && r0 > 0) || (r > 0 && r0 == 0))
	{
		return -1;
	}

	/* We have found nothing */
	if (r == 0 && r0 == 0)
	{
		return 0;
	}

	/* Parse the records into the according structs */

	if ((r = parse_into_passwd(passwd_record, &passwd)) < 0)
	{
		return r;
	}

	if ((r = parse_into_shadow(shadow_record, &shadow)) < 0)
	{
		return r;
	}

	/* Check whether the stored password was empty.
	   If it was, deny access */
	if (shadow.user_hash[0] == '\0')
	{
		return 1;
	}

	/* Compute a hash for the password */
	crypt(password, shadow.user_salt, buf);

	/* The computed hash is equal to that stored in /etc/shadow,
	   grant access */
	if (!strncmp(buf, shadow.user_hash, BUFSIZE))
	{
		/* Access will be granted, reset the current amount
		   of login attempts */
		sys_set_logatt(0);

		/* Set home directory */
		if ((r = chdir(passwd.user_path)) < 0)
		{
			return r;
		}

		if (clear)
		{
			clear_scr();
		}

		/* Begin a session with the user-chosen shell */
		if ((r = spawnl(passwd.user_shell, passwd.user_shell + 1,
			(char *) 0)) < 0)
		{
			return r;
		}

		/* Wait for the session to finish */
		wait(r);

		if (clear)
		{
			clear_scr();
		}

		/* We are done, let init start the login sequence again */
		exit();
	}

	/* Provided login credentials are incorrect */
	return 1;
}

/*
 * Prints the login prompt and reads login and password
 */
void 
prompt(char *login, char *password)
{
	char *buf;

	if (login[0] == '\0')
	{
		buf = readline("login: ");

		if (buf[0] == '\0')
		{
			exit();
		}

		strncpy(login, buf, BUFSIZE);
		login[BUFSIZE - 1] = '\0';
	}

	/* Don't print the password while it is being entered */
	buf = readline_no_echo("password: ");
	strncpy(password, buf, BUFSIZE);
	password[BUFSIZE - 1] = '\0';
}

void
usage(void)
{
	cprintf("Usage: login [-c] [name]\n");
	exit();
}

void
umain(int argc, char *argv[])
{
	int i, r, now;
	unsigned int login_attempts;
	bool clear;
	char login[BUFSIZE], password[BUFSIZE];
	struct Argstate args;

	for (i = 0; i < BUFSIZE; i++)
	{
		login[i] = '\0';
		password[i] = '\0';
	}

	clear = false;
	argstart(&argc, argv, &args);

	/* Read args */
	while ((r = argnext(&args)) >= 0)
	{
		switch(r)
		{
			case 'c':
				clear = true;
				break;
			default:
				usage();
				break;
		}
	}

	/* Should have no more than two args upon reading '-c'
	   (or not reading it at all) */
	if (argc > 2)
	{
		usage();
	}
	/* The login was provided */
	else if (argc == 2)
	{
		strncpy(login, argv[1], BUFSIZE);
	}

	/* Read login/password */
	prompt(login, password);

	/* Try to authenticate with the provided credentials */
	if ((r = auth(login, password, clear)) < 0)
	{
		/* Something serious must have happened */
		panic("login: auth: %i", r);
	}
	else if (r >= 0)
	{
		/* If the authentication was unsuccessful,
		   increase the current amount of login attempts,
		   so that the delay will increase as the
		   unsuccessful attempts continue */

		/* Initialize 'login_attempts' just in case */
		login_attempts = 0;
		/* Get the current amount of login attempts */
		sys_get_logatt(&login_attempts);
		/* Increase the current amount of login attempts exponentially,
		   thus making brute forcing inefficient */
		sys_set_logatt(1 + login_attempts * login_attempts);
		now = vsys_gettime();

		/* Wait for 'login_attempts' seconds */
		while (vsys_gettime() - now <= login_attempts)
		{
		}

		cprintf("Login incorrect\n\n");

		/* The credentials are incorrect,
		   pass the control back to the caller */

		exit();
	}
}
