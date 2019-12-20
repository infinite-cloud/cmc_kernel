#ifndef CRYPT_H
#define CRYPT_H

void crypt(const char *password, const char *salt, char *hash);
void generate_salt(char *hash);

#endif /* !CRYPT_H */
