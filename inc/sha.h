/* Original copyright (C) 2010 by Creytiv.com. */

#ifndef SHA_H
#define SHA_H

/*
 * SHA-1 digest size in bytes.
 */
#define SHA1_DIGEST_SIZE 20
#define SHA_DIGEST_LENGTH SHA1_DIGEST_SIZE

/*
 * This union is intended for internal use by the SHA1_Transform()
 * function.
 */
typedef union
{
	uint8_t c[64];
	uint32_t l[16];
} CHAR64LONG16;

/*
 * SHA-1 context.
 */
typedef struct
{
	uint32_t state[5]; /* Context state */
	uint32_t count[2]; /* Counter */
	uint8_t buffer[64]; /* SHA-1 buffer */
} SHA1_CTX;

/* SHA-1 context (OpenSSL compatibility) */
typedef SHA1_CTX SHA_CTX;

/*
 * These are the helper functions used for generating an SHA1 hash.
 * They are used by the HMAC SHA-1 PRF.
 */
void SHA1_Init(SHA1_CTX *context);
void SHA1_Update(SHA_CTX *context, const void *p, size_t len);
void SHA1_Final(uint8_t digest[SHA1_DIGEST_SIZE], SHA1_CTX *context);

#endif /* !SHA_H */
