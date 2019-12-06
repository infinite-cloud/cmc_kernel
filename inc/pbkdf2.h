/* Original copyright (c) 2000-2001 by Apple Computer, Inc. */

#include <inc/types.h>

#ifndef PBKDF2_H
#define PBKDF2_H

#ifndef UINT32_MAX
#define UINT32_MAX ((1ULL << (8 * sizeof(uint32_t))) - 1)
#endif

/*
 * This function should generate a pseudo random octet stream of hash_len
 * (which is specified as an arument to pbkdf2() and should be constant
 * for any given prf function) bytes long which is output in the buffer
 * pointed to by rnd_ptr (the caller of this function is responsible for
 * allocation of the buffer).
 * The inputs are the first key_len octets pointed to by key_ptr and
 * the first text_len octets pointed to by text_ptr; both lens can have
 * any nonzero value.
 */
typedef void (*PRF)(const uint8_t *key_ptr, size_t key_len,
	const uint8_t *text_ptr, size_t text_len,
	uint8_t *rnd_ptr);

/*
 * This function implements PBKDF2 key derivation algorithm.
 * The output is a derived key of derived_len bytes which is written to
 * the buffer pointed to by derived_ptr.
 * The caller should ensure derived_ptr is at least derived_key bytes long.
 * The key is derived from pwd_ptr (which is pwd_len bytes long) and from
 * salt_ptr (which is salt_len bytes long). it_count iterations are performed.
 * The argument prf is a pointer to a pseudo RNG declared above.
 * It should write exactly hash_len bytes into its output buffer each time it
 * is called. The argument tmp_buffer should point to a buffer
 * MAX(hash_len, salt_len + 4) + 2 * hash_len bytes long. The buffer is used
 * during the calculation for intermediate results.
 * The argument salt_ptr should be a pointer to a buffer of at least 8 random
 * bytes (64 bits). Thus salt_len should be >= 8.
 * For each new session a new salt should be generated.
 * The value of it_cound should be at least 1000.
 */
void pbkdf2(PRF prf, size_t hash_len,
	const void *pwd_ptr, size_t pwd_len,
	const void *salt_ptr, size_t salt_len,
	size_t it_count,
	void *derived_ptr, size_t derived_len,
	void *tmp_buffer);

#endif /* !PBKDF2_H */
