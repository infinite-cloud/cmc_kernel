#include <inc/lib.h>
#include <inc/pbkdf2.h>

void
crypt(const char *password, const char *salt, char *hash)
{
	/* TODO: This is just a placeholder */
	strncpy(hash, password, BUFSIZE);
}
