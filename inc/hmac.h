/* Original copyright (C) 2010 by Creytiv.com. */

#include <inc/types.h>

#ifndef HMAC_H
#define HMAC_H

/*
 * This function implements the HMAC SHA-1 pseudo
 * random hash generator. See the PRF type description
 * in inc/pbkdf2.h for details.
 */
void hmac_sha1(const uint8_t *key_ptr, size_t key_len,
	const uint8_t *text_ptr, size_t text_len,
	uint8_t *rnd_ptr);

#endif /* !HMAC_H */
