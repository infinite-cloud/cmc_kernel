#include <inc/lib.h>

/*
 * Checks whether a string pointed to by record
 * has an appropriate format, i.e.
 * 	...:...:... for /etc/passwd,
 * 	...:...:... for /etc/shadow.
 * members should hold the amount of separators
 * in such string + 1.
 * record should hold a null-terminated buffer
 * of no more than BUFSIZE * members characters
 * long.
 */
static bool
validate_record(const char *record, int members)
{
	int i, separators;

	separators = 0;

	/* Simply calculate the number of separators
	   in a string */
	for (i = 0; i < BUFSIZE * members ; i++)
	{
		if (record[i]== '\0')
		{
			break;
		}

		if (record[i] == SEPARATOR)
		{
			separators++;
		}
	}

	/* If the string is null-terminated and contains the
	   appropriate amount of separators, then we
	   consider it valid */
	return (record[i] == '\0' && separators == members - 1);
}

/*
 * Extracts a name from record and puts it into
 * name.
 * The caller is responsible for checking the validity
 * of record and allocating a buffer of at least
 * BUFSIZE characters for name.
 */
static void
get_name_from_record(char *name, const char *record)
{
	int i;

	for (i = 0; i < BUFSIZE; i++)
	{
		if (record[i] == SEPARATOR)
		{
			name[i] = '\0';
			break;
		}

		name[i] = record[i];
	}
}

int
find_record(int fd, const char *user, char *record, int members)
{
	int i, c, r;
	char buf[BUFSIZE * members], name[BUFSIZE];

	r = 0;

	/* Loop until the file ends or until an error occurs */
	do
	{
		for (i = 0; i < BUFSIZE * members; i++)
		{
			if ((r = read(fd, &c, sizeof(char))) < 0)
			{
				/* The outer loop will terminate afterwards */
				break;
			}

			/* If this is the end of a line,
			   null-terminate the buffer and
			   check it */
			if (c == '\n' || c == '\r' || r == 0)
			{
				buf[i] = '\0';

				/* Validate buffer */
				if (validate_record(buf, members))
				{
					/* If valid, extract the name
					   from the record */
					get_name_from_record(name, buf);

					if (!strncmp(name, user, BUFSIZE))
					{
						/* If it is the record we
						   are looking for, copy it
						   into the result buffer
						   and return */
						strncpy(record, buf, BUFSIZE);
						return 1;
					}
				}

				break;
			}
			else
			{
				/* Add a char to the buffer */
				buf[i] = c;
			}
		}
	} while (r > 0);

	return r;
}

int
parse_into_passwd(const char *record, struct Passwd *passwd)
{
	int i, member, read_count;
	char buf[BUFSIZE * PASSWD_MEMBERS_NUM];
	char *passwd_members[PASSWD_MEMBERS_NUM] =
	{
		passwd->user_name,
		passwd->user_path,
		passwd->user_shell,
	};

	/* Record is invalid, return */
	if (!validate_record(record, PASSWD_MEMBERS_NUM))
	{
		return -1;
	}

	/* This is the amount of chars we read,
	   we will use it as a buffer offset */
	read_count = 0;

	/* Fill up each struct member */
	for (member = 0; member < PASSWD_MEMBERS_NUM; member++)
	{
		/* Store a struct member */
		for (i = 0; i < BUFSIZE; i++)
		{
			/* Struct members are separated by SEPARATORs,
			   and the last one terminates with NULL */
			if (record[read_count + i] == SEPARATOR ||
				record[read_count + i] == '\0')
			{
				/* Null-terminate the buffer and copy it
				   to the corresponding struct member */
				buf[i] = '\0';
				strncpy(passwd_members[member], buf, BUFSIZE);
				break;
			}
			else
			{
				/* Add a char to the buffer */
				buf[i] = record[read_count + i];
			}
		}

		/* We read i chars and added a semicolon */
		read_count += i + 1;
	}

	return 0;
}

int
parse_into_shadow(const char *record, struct Shadow *shadow)
{
	int i, member, read_count;
	char buf[BUFSIZE * SHADOW_MEMBERS_NUM];
	char *shadow_members[SHADOW_MEMBERS_NUM] =
	{
		shadow->user_name,
		shadow->user_salt,
		shadow->user_hash,
	};

	if (!validate_record(record, SHADOW_MEMBERS_NUM))
	{
		return -1;
	}

	read_count = 0;

	for (member = 0; member < SHADOW_MEMBERS_NUM; member++)
	{
		for (i = 0; i < BUFSIZE; i++)
		{
			if (record[read_count + i] == SEPARATOR ||
				record[read_count + i] == '\0')
			{
				buf[i] = '\0';
				strncpy(shadow_members[member], buf, BUFSIZE);
				break;
			}
			else
			{
				buf[i] = record[read_count + i];
			}
		}

		read_count += i + 1;
	}

	return 0;
}

void
parse_from_passwd(char *record, const struct Passwd *passwd)
{
	int member, read_count, len;
	const char *passwd_members[PASSWD_MEMBERS_NUM] =
	{
		passwd->user_name,
		passwd->user_path,
		passwd->user_shell,
	};

	read_count = 0;

	/* Add each struct member to the string, separate them
	   by SEPARATORs, and null-terminate */
	for (member = 0; member < PASSWD_MEMBERS_NUM; member++)
	{
		len = strnlen(passwd_members[member], BUFSIZE);
		strncpy(record + read_count, passwd_members[member], len);
		read_count += len;
		record[read_count++] = (member == PASSWD_MEMBERS_NUM - 1) ?
			'\0' : SEPARATOR;
	}
}

void
parse_from_shadow(char *record, const struct Shadow *shadow)
{
	int member, read_count, len;
	const char *shadow_members[SHADOW_MEMBERS_NUM] =
	{
		shadow->user_name,
		shadow->user_salt,
		shadow->user_hash,
	};

	read_count = 0;

	for (member = 0; member < SHADOW_MEMBERS_NUM; member++)
	{
		len = strnlen(shadow_members[member], BUFSIZE);
		strncpy(record + read_count, shadow_members[member], len);
		read_count += len;
		record[read_count++] = (member == SHADOW_MEMBERS_NUM - 1) ?
			'\0' : SEPARATOR;
	}
}
