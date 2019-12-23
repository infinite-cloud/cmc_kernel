#include <inc/lib.h>

#define BUFSIZ 1024		/* Find the buffer overrun bug! */
int debug_var = 0;


// gettoken(s, 0) prepares gettoken for subsequent calls and returns 0.
// gettoken(0, token) parses a shell token from the previously set string,
// null-terminates that token, stores the token pointer in '*token',
// and returns a token ID (0, '<', '>', '|', or 'w').
// Subsequent calls to 'gettoken(0, token)' will return subsequent
// tokens from the string.
int gettoken(char *s, char **token);


// Parse a shell command from string 's' and execute it.
// Do not return until the shell command is finished.
// runcmd() is called in a forked child,
// so it's OK to manipulate file descriptor state.
#define MAXARGS 16
void
runcmd(char* s)
{
	char *argv[MAXARGS], *t, argv0buf[BUFSIZ], *nf_buf, path[BUFSIZ];
	int argc, c, i, r, p[2], fd, pipe_child;

	pipe_child = 0;
	gettoken(s, 0);

again:
	argc = 0;
	while (1) {
		switch ((c = gettoken(0, &t))) {

		case 'w':	// Add an argument
			if (argc == MAXARGS) {
				cprintf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;

		case '<':	// Input redirection
			// Grab the filename from the argument list
			if (gettoken(0, &t) != 'w') {
				cprintf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading as file descriptor 0
			// (which environments use as standard input).
			// We can't open a file onto a particular descriptor,
			// so open the file as 'fd',
			// then check whether 'fd' is 0.
			// If not, dup 'fd' onto file descriptor 0,
			// then close the original 'fd'.

			// LAB 11: Your code here.
			parse_path(path, t);
			if ((fd = open(path, O_RDONLY)) < 0) {
				cprintf("open %s for read: %i", path, fd);
				exit();
			}
			if (fd != 0) {
				dup(fd, 0);
				close(fd);
			}
			// panic("< redirection not implemented");
			break;

		case '>':	// Output redirection
			// Grab the filename from the argument list
			if (gettoken(0, &t) != 'w') {
				cprintf("syntax error: > not followed by word\n");
				exit();
			}
			parse_path(path, t);
			if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC)) < 0) {
				cprintf("open %s for write: %i", path, fd);
				exit();
			}
			if (fd != 1) {
				dup(fd, 1);
				close(fd);
			}
			break;

		case '|':	// Pipe
			if ((r = pipe(p)) < 0) {
				cprintf("pipe: %i", r);
				exit();
			}
			if (debug_var)
				cprintf("PIPE: %d %d\n", p[0], p[1]);
			if ((r = fork()) < 0) {
				cprintf("fork: %i", r);
				exit();
			}
			if (r == 0) {
				if (p[0] != 0) {
					dup(p[0], 0);
					close(p[0]);
				}
				close(p[1]);
				goto again;
			} else {
				pipe_child = r;
				if (p[1] != 1) {
					dup(p[1], 1);
					close(p[1]);
				}
				close(p[0]);
				goto runit;
			}
			panic("| not implemented");
			break;

		case 0:		// String is complete
			// Run the current command!
			goto runit;

		default:
			panic("bad return %d from gettoken", c);
			break;

		}
	}

runit:
	// Return immediately if command line was empty.
	if(argc == 0) {
		if (debug_var)
			cprintf("EMPTY COMMAND\n");
		return;
	}

	// Save the current command in case we won't find it in the cwd
	nf_buf = argv[0];

	// Clean up command line.
	// Read all commands from the filesystem: add an initial '/' to
	// the command name.
	// This essentially acts like 'PATH=/'.

	// Write the cwd to argv0buf instead.
	parse_path(argv0buf, argv[0]);
	argv[0] = argv0buf;
	argv[argc] = 0;

	// Print the command.
	if (debug_var) {
		cprintf("[%08x] SPAWN:", thisenv->env_id);
		for (i = 0; argv[i]; i++)
			cprintf(" %s", argv[i]);
		cprintf("\n");
	}

	// Change the directory and return if this is 'cd'
	if (!strncmp(argv[0], "/cd", BUFSIZ)) {
		if (argc != 2) {
			cprintf("Usage: cd PATH\n");
			return;
		}

		if ((r = chdir(argv[1])) < 0) {
			cprintf("cd: %i\n", r);
		}

		return;
	}

	// Print the cwd and return if this is 'pwd'
	if (!strncmp(argv[0], "/pwd", BUFSIZ)) {
		if (argc != 1) {
			cprintf("Usage: pwd\n");
			return;
		}

		getcwd(path);
		cprintf("%s\n", path);
		return;
	}

	// Save the cwd to restore it in case of a context switch
	getcwd(path);
	// Spawn the command!
	if ((r = spawn(argv[0], (const char**) argv)) < 0) {
		// We haven't found the command in the cwd,
		// look it up in the root directory
		if (r == -E_NOT_FOUND) {
			argv0buf[0] = '/';
			strcpy(argv0buf + 1, nf_buf);
			argv[0] = argv0buf;

			if (!strncmp(argv[0], "/cd", BUFSIZ)) {
				if (argc != 2) {
					cprintf("Usage: cd PATH\n");
					return;
				}

				if ((r = chdir(argv[1])) < 0) {
					cprintf("cd: %i\n", r);
				}

				return;
			}

			if (!strncmp(argv[0], "/pwd", BUFSIZ)) {
				if (argc != 1) {
					cprintf("Usage: pwd\n");
					return;
				}

				cprintf("%s\n", path);
				return;
			}

			if ((r = spawn(argv[0], (const char**) argv)) < 0) {
				cprintf("spawn %s: %i\n", argv[0], r);
			}
		}
	}

	// In the parent, close all file descriptors and wait for the
	// spawned command to exit.
	close_all();
	if (r >= 0) {
		if (debug_var)
			cprintf("[%08x] WAIT %s %08x\n", thisenv->env_id, argv[0], r);
		wait(r);
		// The child has finished, we can restore the cwd now
		chdir(path);
		if (debug_var)
			cprintf("[%08x] wait finished\n", thisenv->env_id);
	}

	// If we were the left-hand part of a pipe,
	// wait for the right-hand part to finish.
	if (pipe_child) {
		if (debug_var)
			cprintf("[%08x] WAIT pipe_child %08x\n", thisenv->env_id, pipe_child);
		wait(pipe_child);
		if (debug_var)
			cprintf("[%08x] wait finished\n", thisenv->env_id);
	}

	// Done!
	exit();
}


// Get the next token from string s.
// Set *p1 to the beginning of the token and *p2 just past the token.
// Returns
//	0 for end-of-string;
//	< for <;
//	> for >;
//	| for |;
//	w for a word.
//
// Eventually (once we parse the space where the \0 will go),
// words get nul-terminated.
#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

int
_gettoken(char *s, char **p1, char **p2)
{
	int t;

	if (s == 0) {
		if (debug_var > 1)
			cprintf("GETTOKEN NULL\n");
		return 0;
	}

	if (debug_var > 1)
		cprintf("GETTOKEN: %s\n", s);

	*p1 = 0;
	*p2 = 0;

	while (strchr(WHITESPACE, *s))
		*s++ = 0;
	if (*s == 0) {
		if (debug_var > 1)
			cprintf("EOL\n");
		return 0;
	}
	if (strchr(SYMBOLS, *s)) {
		t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		if (debug_var > 1)
			cprintf("TOK %c\n", t);
		return t;
	}
	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s))
		s++;
	*p2 = s;
	if (debug_var > 1) {
		t = **p2;
		**p2 = 0;
		cprintf("WORD: %s\n", *p1);
		**p2 = t;
	}
	return 'w';
}

int
gettoken(char *s, char **p1)
{
	static int c, nc;
	static char* np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}


// Check if there is an 'exit' command in the buffer
int
try_exit(const char *buf)
{
	int i;

	i = 0;

	// Skip the whitespaces in the beginning
	while (i < BUFSIZ && strchr(WHITESPACE, buf[i])) {
		i++;
	}

	// Try to find an 'exit' sequence
	if (i < BUFSIZ - 4 && !strncmp(&buf[i], "exit", 4)) {
		i += 4;

		// If 'exit' is followed by some non-whitespace symbol,
		// it is some other command
		if (buf[i] != '\0' && !strchr(WHITESPACE, buf[i++])) {
			return 0;
		}

		// Skip the next set of whitespaces
		while (i < BUFSIZ) {
			// It is indeed an 'exit' without any arguments
			if (buf[i] == '\0') {
				break;
			}

			// It is 'exit', but it is followed by some arguments
			if (!strchr(WHITESPACE, buf[i++])) {
				cprintf("Usage: exit\n");
				return -1;
			}
		}

		// If this is just an 'exit', exit
		exit();
	}

	// This was definitely not an 'exit' sequence, we have found nothing
	return 0;
}

void
usage(void)
{
	cprintf("usage: sh [-dix] [command-file]\n");
	exit();
}

void
umain(int argc, char **argv)
{
	int r, interactive, echocmds;
	struct Argstate args;

	interactive = '?';
	echocmds = 0;
	argstart(&argc, argv, &args);
	while ((r = argnext(&args)) >= 0)
		switch (r) {
		case 'd':
			debug_var++;
			break;
		case 'i':
			interactive = 1;
			break;
		case 'x':
			echocmds = 1;
			break;
		default:
			usage();
		}

	if (argc > 2)
		usage();
	if (argc == 2) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0)
			panic("open %s: %i", argv[1], r);
		assert(r == 0);
	}
	if (interactive == '?')
		interactive = iscons(0);

	while (1) {
		char *buf;

		buf = readline(interactive ? "$ " : NULL);
		if (buf == NULL) {
			if (debug_var)
				cprintf("EXITING\n");
			exit();	// end of file
		}
		if (debug_var)
			cprintf("LINE: %s\n", buf);
		if (buf[0] == '#')
			continue;
		if (echocmds)
			printf("# %s\n", buf);
		// Check whether this is an 'exit' command and exit if it is
		// before moving on to fork. If it is an 'exit' but followed by
		// some arguments, try again
		if (try_exit(buf) < 0)
			continue;
		if (debug_var)
			cprintf("BEFORE FORK\n");
		if ((r = fork()) < 0)
			panic("fork: %i", r);
		if (debug_var)
			cprintf("FORK: %d\n", r);
		if (r == 0) {
			runcmd(buf);
			exit();
		} else
			wait(r);
	}
}

