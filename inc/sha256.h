#ifndef SHA256_H
#define SHA256_H

#include <inc/types.h>

/*
 * Defines the size of a SHA-256 hash value in bits
 */
#define SHA256_HASH_BITS  256

/*
 * Defines the size of a SHA-256 hash value in bytes
 */
#define SHA256_HASH_BYTES (SHA256_HASH_BITS / 8)

/*
 * Defines the size of a SHA-256 input block in bits
 */
#define SHA256_BLOCK_BITS 512

/*
 * Defines the size of a SHA-256 input block in bytes
 */
#define SHA256_BLOCK_BYTES (SHA256_BLOCK_BITS / 8)

/*
 * SHA-256 context type
 *
 * A variable of this type may hold the state of a SHA-256 hashing process
 */
typedef struct
{
	uint32_t h[8];
	uint64_t length;
} sha256_ctx_t;

/*
 * SHA-256 hash value type
 *
 * A variable of this type may hold the hash value produced by the
 * sha256_ctx2hash(sha256_hash_t *dest, const sha256_ctx_t *state) function.
 */
typedef uint8_t sha256_hash_t[SHA256_HASH_BYTES];

/*
 * Initialise a SHA-256 context
 *
 * This function sets a ::sha256_ctx_t to the initial values for hashing.
 * 'state' pointer to the SHA-256 hashing context
 */
void sha256_init(sha256_ctx_t *state);

/*
 * Update the context with a given block
 *
 * This function updates the SHA-256 hash context by processing the given block
 * of fixed length.
 * 'state' pointer to the SHA-256 hash context
 * 'block' pointer to the block of fixed length (512 bit = 64 byte)
 */
void sha256_nextblock (sha256_ctx_t *state, const void *block);

/*
 * Finalize the context with the given block
 *
 * This function finalizes the SHA-256 hash context by processing the given
 * block of variable length.
 * 'state' pointer to the SHA-256 hash context
 * 'block' pointer to the block of fixed length (512 bit = 64 byte)
 * 'length_b' the length of the block in bits
 */
void sha256_lastblock(sha256_ctx_t *state, const void *block,
	uint16_t length_b);

/*
 * Convert the hash state into the hash value
 *
 * This function reads the context and writes the hash value to the destination
 * 'dest' pointer to the location where the hash value should be written
 * 'state' pointer to the SHA-256 hash context
 */
void sha256_ctx2hash(sha256_hash_t *dest, const sha256_ctx_t *state);

/*
 * Simple SHA-256 hashing function for direct hashing
 *
 * This function automaticaly hashes a given message of arbitary length with
 * the SHA-256 hashing algorithm.
 * 'dest' pointer to the location where the hash value is going to be written to
 * 'msg' pointer to the message thats going to be hashed
 * 'length_b' length of the message in bits
 */
void sha256(sha256_hash_t *dest, const void *msg, uint32_t length_b);

#endif /* !SHA256_H */
