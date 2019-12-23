#include <inc/lib.h>

char buf[8192];

void
cat(int f, char *s)
{
	long n;
	int r;

	while ((n = read(f, buf, (long)sizeof(buf))) > 0)
		if ((r = write(1, buf, n)) != n)
			panic("write error copying %s: %i", s, r);
	if (n < 0)
		panic("error reading %s: %i", s, (int) n);
}

void
umain(int argc, char **argv)
{
	int f, i;
	size_t len;

	binaryname = "cat";
	if (argc == 1)
		cat(0, "<stdin>");
	else
		for (i = 1; i < argc; i++) {
			if (argv[i][0] != '/') {
				getcwd(buf);
				len = strnlen(buf, 8192 - 1);
				if (len > 1)
					strcat(buf, "/");
				strcat(buf, argv[i]);
				argv[i] = buf;
			}

			f = open(argv[i], O_RDONLY);
			if (f < 0)
				printf("can't open %s: %i\n", argv[i], f);
			else {
				cat(f, argv[i]);
				close(f);
			}
		}
}
