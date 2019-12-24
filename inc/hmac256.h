#ifndef HMACSHA256_H
#define HMACSHA256_H

#include <inc/sha256.h>

#define HMAC_SHA256_BITS        SHA256_HASH_BITS
#define HMAC_SHA256_BYTES       SHA256_HASH_BYTES
#define HMAC_SHA256_BLOCK_BITS  SHA256_BLOCK_BITS
#define HMAC_SHA256_BLOCK_BYTES SHA256_BLOCK_BYTES

typedef struct
{
	sha256_ctx_t a, b;
} hmac_sha256_ctx_t;

void hmac_sha256_init(hmac_sha256_ctx_t *s, const void *key,
	uint16_t keylength_b);
void hmac_sha256_nextblock(hmac_sha256_ctx_t *s, const void *block);
void hmac_sha256_lastblock(hmac_sha256_ctx_t *s, const void *block,
	uint16_t length_b);
void hmac_sha256_final(void *dest, hmac_sha256_ctx_t *s);

void hmac_sha256(const uint8_t *key, size_t keylength_b, const uint8_t *msg,
	size_t msglength_b, uint8_t *dest);

#endif /* !HMACSHA256_H */
