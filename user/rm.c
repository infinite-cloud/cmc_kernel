#include <inc/lib.h>

void
usage()
{
	cprintf("Usage: rm [-f] [-r]\n");
	exit();
}

void
umain(int argc, char *argv[])
{
	int r;
	char buf[BUFSIZE];
	bool rm_folder, ignore_all;
	struct Argstate args;
	struct Stat st;

	rm_folder = false;
	ignore_all = false;
	argstart(&argc, argv, &args);

	/* Read args */
	while ((r = argnext(&args)) >= 0)
	{
		switch(r)
		{
			case 'r':
				rm_folder = true;
				break;
			case 'f':
				ignore_all = true;
				break;
			default:
				usage();
				break;
		}
	}

	/* rm should have at least one arg */
	if (argc < 2)
	{
		usage();
	}

	/* Loop through args and try to delete each one */
	while (argc > 1)
	{
		/* Get an absolute path */
		parse_path(buf, argv[argc - 1]);

		/* Ignore any errors if the flag is set */
		if ((r = stat(buf, &st)) < 0 && !ignore_all)
		{
			cprintf("rm: invalid file '%s'\n", argv[argc - 1]);
			exit();
		}

		/* Can only delete a folder if the corresponding flag is set */
		if (st.st_isdir && !rm_folder)
		{
			cprintf("rm: '%s' is a folder\n", argv[argc - 1]);
			exit();
		}

		/* Delete it */
		remove(buf);
		/* Move on to the next arg */
		argc--;
	}
}
