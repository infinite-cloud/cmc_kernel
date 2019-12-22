#include <inc/lib.h>
#include <inc/pbkdf2.h>
#include <inc/hmac.h>
#include <inc/random.h>

#define PBKDF2_ITERATIONS 1000

static const char encoding_table[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

static void
base64_encode(char *out, size_t *out_len, const uint8_t *in, size_t in_len)
{
	int i, j;
	uint32_t oct_a, oct_b, oct_c, triple;
	const int mod_table[] = {0, 2, 1};

	*out_len = 4 * ((in_len + 2) / 2);

	for (i = 0, j = 0; i < in_len; )
	{
		oct_a = (i < in_len) ? (uint8_t) in[i++] : 0;
		oct_b = (i < in_len) ? (uint8_t) in[i++] : 0;
		oct_c = (i < in_len) ? (uint8_t) in[i++] : 0;

		triple = (oct_a << 0x10) + (oct_b << 0x08) + (oct_c);

		out[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		out[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		out[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		out[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (i = 0; i < mod_table[in_len % 3]; i++)
	{
		out[*out_len - 1 - i] = '=';
	}
}

void
crypt(const char *password, const char *salt, char *hash)
{
	size_t hash_len;
	uint8_t buf[HASH_LENGTH + 2 * SHA_DIGEST_LENGTH];
	uint8_t derived_key[HASH_LENGTH];

	pbkdf2(hmac_sha1, SHA_DIGEST_LENGTH,
		(const void *) password, strnlen(password, BUFSIZE),
		(const void *) salt, SALT_LENGTH,
		PBKDF2_ITERATIONS,
		(void *) derived_key, HASH_LENGTH,
		(void *) buf);

	hash_len = 0;
	base64_encode(hash, &hash_len, derived_key, HASH_LENGTH);
	hash[hash_len] = '\0';
}

void
generate_salt(char *salt)
{
	int i, len;

	rand_init(0);
	srand(vsys_gettime());
	len = strlen(encoding_table) - 1;

	for (i = 0; i < SALT_LENGTH; i++)
	{
		salt[i] = encoding_table[rand() % len];
	}

	salt[SALT_LENGTH] = '\0';
}
