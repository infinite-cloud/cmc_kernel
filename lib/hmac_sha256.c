#include <inc/types.h>
#include <inc/string.h>
#include <inc/sha256.h>
#include <inc/hmac256.h>

#define IPAD 0x36
#define OPAD 0x5C

#ifndef HMAC_SHA256_SHORTONLY

void hmac_sha256_init(hmac_sha256_ctx_t *s, const void *key,
	uint16_t keylength_b)
{
	uint8_t buffer[HMAC_SHA256_BLOCK_BYTES];
	uint8_t i;

	memset(buffer, 0, HMAC_SHA256_BLOCK_BYTES);

	if (keylength_b > HMAC_SHA256_BLOCK_BITS)
	{
		sha256((void *) buffer, key, keylength_b);
	}
	else
	{
		memcpy(buffer, key, (keylength_b + 7) / 8);
	}

	for (i = 0; i < HMAC_SHA256_BLOCK_BYTES; i++)
	{
		buffer[i] ^= IPAD;
	}

	sha256_init(&(s->a));
	sha256_nextblock(&(s->a), buffer);

	for (i = 0; i < HMAC_SHA256_BLOCK_BYTES; i++)
	{
		buffer[i] ^= IPAD ^ OPAD;
	}

	sha256_init(&(s->b));
	sha256_nextblock(&(s->b), buffer);
}

void hmac_sha256_nextblock(hmac_sha256_ctx_t *s, const void *block)
{
	sha256_nextblock(&(s->a), block);
}

void hmac_sha256_lastblock(hmac_sha256_ctx_t *s, const void *block, uint16_t length_b)
{
	sha256_lastblock(&(s->a), block, length_b);
}

void hmac_sha256_final(void *dest, hmac_sha256_ctx_t *s)
{
	sha256_ctx2hash((sha256_hash_t *) dest, &(s->a));
	sha256_lastblock(&(s->b), dest, SHA256_HASH_BITS);
	sha256_ctx2hash((sha256_hash_t *) dest, &(s->b));
}

#endif

/*
 * keylength in bits
 * message length in bits
 *
 * a one-shot
 */
void hmac_sha256(const uint8_t *key, size_t keylength_b, const uint8_t *msg,
	size_t msglength_b, uint8_t *dest)
{
	sha256_ctx_t s;
	uint8_t i;
	uint8_t buffer[HMAC_SHA256_BLOCK_BYTES];

	memset(buffer, 0, HMAC_SHA256_BLOCK_BYTES);

	/* If key is larger than a block, we have to hash it */
	if (keylength_b > SHA256_BLOCK_BITS)
	{
		sha256((void *) buffer, key, keylength_b);
	}
	else
	{
		memcpy(buffer, key, (keylength_b + 7) / 8);
	}

	for (i = 0; i < SHA256_BLOCK_BYTES; i++)
	{
		buffer[i] ^= IPAD;
	}

	sha256_init(&s);
	sha256_nextblock(&s, buffer);

	while (msglength_b >= HMAC_SHA256_BLOCK_BITS)
	{
		sha256_nextblock(&s, msg);
		msg = (uint8_t *) msg + HMAC_SHA256_BLOCK_BYTES;
		msglength_b -= HMAC_SHA256_BLOCK_BITS;
	}

	sha256_lastblock(&s, msg, msglength_b);

	/* Since buffer still contains key xor ipad, we can do ... */
	for (i = 0; i < HMAC_SHA256_BLOCK_BYTES; i++)
	{
		buffer[i] ^= IPAD ^ OPAD;
	}

	/* Save inner hash temporary to dest */
	sha256_ctx2hash((void *) dest, &s);
	sha256_init(&s);
	sha256_nextblock(&s, buffer);
	sha256_lastblock(&s, dest, SHA256_HASH_BITS);
	sha256_ctx2hash((void *) dest, &s);
}
