#include <inc/types.h>

#ifndef PBKDF2_H
#define PBKDF2_H

#ifndef UINT32_MAX
#define UINT32_MAX ((1ULL << (8 * sizeof(uint32_t))) - 1)
#endif

typedef void (*PRF)(const uint8_t *key_ptr, size_t key_len,
	const uint8_t *text_ptr, size_t text_len,
	uint8_t *rnd_ptr);

void pbkdf2(PRF prf, size_t hash_len,
	const void *pwd_ptr, size_t pwd_len,
	const void *salt_ptr, size_t salt_len,
	size_t it_count,
	void *derived_ptr, size_t derived_len,
	void *tmp_buffer);

#endif /* !PBKDF2_H */
