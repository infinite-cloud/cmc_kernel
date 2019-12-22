#ifndef SHA_H
#define SHA_H

#define SHA1_DIGEST_SIZE 20
#define SHA_DIGEST_LENGTH SHA1_DIGEST_SIZE

typedef union
{
	uint8_t c[64];
	uint32_t l[16];
} CHAR64LONG16;

typedef struct
{
	uint32_t state[5];
	uint32_t count[2];
	uint8_t buffer[64];
} SHA1_CTX;

typedef SHA1_CTX SHA_CTX;

void SHA1_Init(SHA1_CTX *context);
void SHA1_Update(SHA_CTX *context, const void *p, size_t len);
void SHA1_Final(uint8_t digest[SHA1_DIGEST_SIZE], SHA1_CTX *context);

#endif /* !SHA_H */
