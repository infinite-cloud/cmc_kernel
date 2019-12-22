#include <inc/lib.h>

struct {
	char msg1[5000];
	char msg2[1000];
} data = {
	"this is initialized data",
	"so is this"
};

char bss[6000];

int
sum(const char *s, int n)
{
	int i, tot = 0;
	for (i = 0; i < n; i++)
		tot ^= i * s[i];
	return tot;
}

int
login_init(void)
{
	int i, fd, r;
	const char *paths[] =
	{
		"/etc",
		"/etc/passwd",
		"/etc/shadow",
		"/home",
		"/home/user",
		"/tmp",
	};
	const unsigned int modes[] =
	{
		O_CREAT | O_EXCL | O_MKDIR,
		O_CREAT | O_EXCL,
		O_CREAT | O_EXCL,
		O_CREAT | O_EXCL | O_MKDIR,
		O_CREAT | O_EXCL | O_MKDIR,
		O_CREAT | O_EXCL | O_MKDIR,
	};
	const char *default_user[] =
	{
		"",
		"user:/home/user:/sh",
		"user:osPUWC4ITJ92:m9NMSz2HozU1OYGKaaEWdydNVXEA",
		"",
		"",
		"",
	};

	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
	{
		if ((r = open(paths[i], modes[i])) < 0 && r != -E_FILE_EXISTS)
		{
			return r;
		}

		if (i == 1 || i == 2)
		{
			fd = open(paths[i], O_WRONLY);

			if ((r = write(fd, (const void *) default_user[i],
				strlen(default_user[i]) * sizeof(char))) < 0)
			{
				close(fd);
				return r;
			}

			close(fd);
		}
	}

	return 0;
}

void
umain(int argc, char **argv)
{
	int i, r, x, want;
	char args[256];

	cprintf("init: running\n");

	want = 0xf989e;
	if ((x = sum((char*)&data, sizeof data)) != want)
		cprintf("init: data is not initialized: got sum %08x wanted %08x\n",
			x, want);
	else
		cprintf("init: data seems okay\n");
	if ((x = sum(bss, sizeof bss)) != 0)
		cprintf("bss is not initialized: wanted sum 0 got %08x\n", x);
	else
		cprintf("init: bss seems okay\n");

	// output in one syscall per line to avoid output interleaving 
	strcat(args, "init: args:");
	for (i = 0; i < argc; i++) {
		strcat(args, " '");
		strcat(args, argv[i]);
		strcat(args, "'");
	}
	cprintf("%s\n", args);

	// cprintf("init: running sh\n");
	cprintf("init: running login\n");

	// being run directly from kernel, so no file descriptors open yet
	close(0);
	if ((r = opencons()) < 0)
		panic("opencons: %i", r);
	if (r != 0)
		panic("first opencons used fd %d", r);
	if ((r = dup(0, 1)) < 0)
		panic("dup: %i", r);
	if ((r = login_init()) < 0)
		panic("login_init: %i", r);
	while (1) {
		// cprintf("init: starting sh\n");
		cprintf("init: starting login\n");
		// r = spawnl("/sh", "sh", (char*)0);
		r = spawnl("/login", "login", "-c", (char *) 0);
		if (r < 0) {
			// cprintf("init: spawn sh: %i\n", r);
			cprintf("init: spawn login: %i\n", r);
			continue;
		}
		wait(r);
	}
}
