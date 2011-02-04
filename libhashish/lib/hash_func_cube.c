#include "libhashish.h"

#include <string.h>

/* Cube Hash by DJB (and Students?)
 * URL: http://cubehash.cr.yp.to/software.html
 */

#define CUBEHASH_ROUNDS 8
#define CUBEHASH_BLOCKBYTES 1

typedef unsigned char BitSequence;
typedef unsigned long long DataLength;

typedef struct {
  int hashbitlen;
  int pos; /* number of bits read into x from current block */
  uint32_t x[32];
} hashState;

#define ROTATE(a,b) (((a) << (b)) | ((a) >> (32 - b)))

static void transform(hashState *state)
{
  int i;
  int r;
  uint32_t y[16];

  for (r = 0;r < CUBEHASH_ROUNDS;++r) {
    for (i = 0;i < 16;++i) state->x[i + 16] += state->x[i];
    for (i = 0;i < 16;++i) y[i ^ 8] = state->x[i];
    for (i = 0;i < 16;++i) state->x[i] = ROTATE(y[i],7);
    for (i = 0;i < 16;++i) state->x[i] ^= state->x[i + 16];
    for (i = 0;i < 16;++i) y[i ^ 2] = state->x[i + 16];
    for (i = 0;i < 16;++i) state->x[i + 16] = y[i];
    for (i = 0;i < 16;++i) state->x[i + 16] += state->x[i];
    for (i = 0;i < 16;++i) y[i ^ 4] = state->x[i];
    for (i = 0;i < 16;++i) state->x[i] = ROTATE(y[i],11);
    for (i = 0;i < 16;++i) state->x[i] ^= state->x[i + 16];
    for (i = 0;i < 16;++i) y[i ^ 1] = state->x[i + 16];
    for (i = 0;i < 16;++i) state->x[i + 16] = y[i];
  }
}

static int Init(hashState *state, int hashbitlen)
{
  int i;

  if (hashbitlen < 8) return 2;
  if (hashbitlen > 512) return 2;
  if (hashbitlen != 8 * (hashbitlen / 8)) return 2;

  state->hashbitlen = hashbitlen;
  for (i = 0;i < 32;++i) state->x[i] = 0;
  state->x[0] = hashbitlen / 8;
  state->x[1] = CUBEHASH_BLOCKBYTES;
  state->x[2] = CUBEHASH_ROUNDS;
  for (i = 0;i < 10;++i) transform(state);
  state->pos = 0;
  return 0;
}

static int Update(hashState *state, const BitSequence *data,
                  DataLength databitlen)
{
  /* caller promises us that previous data had integral number of bytes */
  /* so state->pos is a multiple of 8 */

  while (databitlen >= 8) {
    uint32_t u = *data;
    u <<= 8 * ((state->pos / 8) % 4);
    state->x[state->pos / 32] ^= u;
    data += 1;
    databitlen -= 8;
    state->pos += 8;
    if (state->pos == 8 * CUBEHASH_BLOCKBYTES) {
      transform(state);
      state->pos = 0;
    }
  }
  if (databitlen > 0) {
    uint32_t u = *data;
    u <<= 8 * ((state->pos / 8) % 4);
    state->x[state->pos / 32] ^= u;
    state->pos += databitlen;
  }
  return 0;
}

static int Final(hashState *state, BitSequence *hashval)
{
  int i;
  uint32_t u;

  u = (128 >> (state->pos % 8));
  u <<= 8 * ((state->pos / 8) % 4);
  state->x[state->pos / 32] ^= u;
  transform(state);
  state->x[31] ^= 1;
  for (i = 0;i < 10;++i) transform(state);
  for (i = 0;i < state->hashbitlen / 8;++i) hashval[i] = state->x[i / 4] >> (8 * (i % 4));

  return 0;
}

static int Hash(int hashbitlen, const BitSequence *data,
                DataLength databitlen, BitSequence *hashval)
{
  hashState state;
  if (Init(&state,hashbitlen) != 0) return 2;
  Update(&state,data,databitlen);
  return Final(&state,hashval);
}

#define	CUBE_BLOCK_SIZE 28


uint32_t lhi_hash_cube(const uint8_t *data, uint32_t len)
{
	uint32_t ret;
	uint8_t hash[CUBE_BLOCK_SIZE];

	/**
	 * Block sizes for SHA-3:
	 *
	 *	28  - SHA3_224
	 *	32  - SHA3_256
	 *	48  - SHA3_384
	 *	64  - SHA3_512
	 *	128 - SHA3_1024
	 */
	Hash(CUBE_BLOCK_SIZE * 8, data, len * 8, hash);

	/* truncate hash - consider the 32 higher order bits */
	memcpy(&ret, hash, sizeof(ret));

	return ret;
}
