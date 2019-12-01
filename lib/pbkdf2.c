#include <inc/string.h>
#include <inc/pbkdf2.h>

static void compute_pbkdf2(PRF prf, size_t hash_len,
	const void *pwd_ptr, size_t pwd_len,
	const void *salt_ptr, size_t salt_len,
	size_t it_count, uint32_t block_num,
	void *data_ptr,	void *tmp_buffer)
{
	uint8_t *in_block, *out_block, *result_block;
	uint8_t *tmp_block;
	uint32_t byte;
	size_t i;

	out_block = (uint8_t *) tmp_buffer;
	in_block = out_block + hash_len;

	memcpy(in_block, salt_ptr, salt_len);

	in_block[salt_len + 0] = (uint8_t) (block_num >> 24);
	in_block[salt_len + 1] = (uint8_t) (block_num >> 16);
	in_block[salt_len + 2] = (uint8_t) (block_num >> 8);
	in_block[salt_len + 3] = (uint8_t) (block_num);

	result_block = (uint8_t *) data_ptr;
	prf(pwd_ptr, pwd_len, in_block, salt_len + 4, out_block);
	memcpy(result_block, out_block, hash_len);

	for (i = 2; i <= it_count; i++)
	{
		tmp_block = in_block;
		in_block = out_block;
		out_block = tmp_block;

		prf(pwd_ptr, pwd_len, in_block, hash_len, out_block);

		for (byte = 0; byte < hash_len; byte++)
		{
			result_block[byte] ^= out_block[byte];
		}
	}
}

void pbkdf2(PRF prf, size_t hash_len,
	const void *pwd_ptr, size_t pwd_len,
	const void *salt_ptr, size_t salt_len,
	size_t it_count,
	void *derived_ptr, size_t derived_len,
	void *tmp_buffer)
{
	size_t complete_blocks, partial_size;
	uint32_t block_num;
	uint8_t *data_ptr, *blk_buffer;

	complete_blocks = derived_len / hash_len;
	partial_size = derived_len % hash_len;
	data_ptr = (uint8_t *) derived_ptr;
	blk_buffer = (uint8_t *) tmp_buffer;

	complete_blocks &= UINT32_MAX;

	for (block_num = 1; block_num <= complete_blocks; block_num++)
	{
		compute_pbkdf2(prf, hash_len, pwd_ptr, pwd_len, salt_ptr,
			salt_len, it_count, block_num, data_ptr,
			blk_buffer + hash_len);
		data_ptr += hash_len;
	}

	if (partial_size > 0)
	{
		compute_pbkdf2(prf, hash_len, pwd_ptr, pwd_len, salt_ptr,
			salt_len, it_count, block_num, data_ptr,
			blk_buffer + hash_len);
		memcpy(data_ptr, blk_buffer, partial_size);
	}
}
