#include <inc/sha.h>

#ifndef CRYPT_H
#define CRYPT_H

#define HASH_LENGTH SHA_DIGEST_LENGTH
#define SALT_LENGTH 12

void crypt(const char *password, const char *salt, char *hash);
void generate_salt(char *hash);

#endif /* !CRYPT_H */
