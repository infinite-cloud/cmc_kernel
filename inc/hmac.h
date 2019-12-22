#include <inc/types.h>

#ifndef HMAC_H
#define HMAC_H

void hmac_sha1(const uint8_t *key_ptr, size_t key_len,
	const uint8_t *text_ptr, size_t text_len,
	uint8_t *rnd_ptr);

#endif /* !HMAC_H */
