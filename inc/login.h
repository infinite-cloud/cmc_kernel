#ifndef LOGIN_H
#define LOGIN_H

#define BUFSIZE 1024
#define PASSWD_MEMBERS_NUM 3
#define SHADOW_MEMBERS_NUM 3
#define SEPARATOR ':'

/*
 * This struct is used to store a parsed record from
 * /etc/passwd.
 */
struct Passwd
{
	char user_name[BUFSIZE];
	char user_path[BUFSIZE];
	char user_shell[BUFSIZE];
};

/*
 * This struct is used to store a parsed record from
 * /etc/shadow.
 */
struct Shadow
{
	char user_name[BUFSIZE];
	char user_salt[BUFSIZE];
	char user_hash[BUFSIZE];
};

/*
 * This function looks up a valid record that contains a name user
 * in a file distinguished by file descriptor fd.
 * fd should be a file descriptor of a file open for reading.
 * user should contain a null-terminated string.
 * The caller is responsible for allocating a buffer of at least
 * BUFSIZE * members characters long for record.
 * members is used for a validity check and should contain
 * the amount of members of struct Passwd / struct Shadow, i.e.
 * 	...:...:... for /etc/passwd,
 * 	...:...:... for /etc/shadow.
 * Returns:
 * 	positive if the record is found,
 * 	negative if an error occurs,
 * 	zero if the record is not found.
 */
int find_record(int fd, const char *user, char *record, int members);
/*
 * The following two functions parse a string pointed to by record
 * and fill up the members of passwd (shadow).
 * The caller is responsible for providing a non-null pointer
 * to struct Passwd (struct Shadow).
 * record should be a null-terminated buffer of no more than
 * BUFSIZE * PASSWD_MEMBER_NUM (BUFSIZE * SHADOW_MEMBER_NUM).
 * Returns:
 * 	negative if record is not valid,
 * 	positive otherwise.
 */
int parse_into_passwd(const char *record, struct Passwd *passwd);
int parse_into_shadow(const char *record, struct Shadow *shadow);
/* The following two functions do the opposite to the previous two.
 * The caller is resposible for allocating a buffer of at least
 * BUFSIZE * PASSWD_MEMBER_NUM (BUFSIZE * SHADOW_MEMBER_NUM)
 * for record.
 * passwd (shadow) should be a non-null pointer to the corresponding
 * struct.
 */
void parse_from_passwd(char *record, const struct Passwd *passwd);
void parse_from_shadow(char *record, const struct Shadow *shadow);

#endif /* !LOGIN_H */
