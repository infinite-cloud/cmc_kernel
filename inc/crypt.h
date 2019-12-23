#include <inc/sha.h>

#ifndef CRYPT_H
#define CRYPT_H

#define HASH_LENGTH SHA_DIGEST_LENGTH
#define SALT_LENGTH 12

/*
 * This function generates a base-64 PBKDF2 hash based on the provided
 * salt and password.
 * 'password' and 'salt' should be null-terminated character strings
 * satisfying the conditions for the pbkdf2() function.
 * The caller is responsible for allocating a buffer at least HASH_LENGTH + 1
 * bytes long for 'hash'. The output string is null-terminated.
 */
void crypt(const char *password, const char *salt, char *hash);
/*
 * This function generates a pseudo random salt SALT_LENGTH
 * bytes long in a base-64 format. The output string is null-terminated.
 * The caller is responsible for allocating a buffer at least SALT_LENGTH + 1
 * bytes long.
 */
void generate_salt(char *hash);

#endif /* !CRYPT_H */
