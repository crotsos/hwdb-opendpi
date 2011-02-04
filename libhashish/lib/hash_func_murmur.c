/*
 * MurmurHash2Aligned, By Austin Appleby (aappleby (AT) gmail).
 * Source code is released to the public domain.
 * taken from http://murmurhash.googlepages.com/
 *
 * Same algorithm as MurmurHash, but only does aligned reads - should be safer
 * on certain platforms.
 * And it has a few limitations -
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian
 */

#include "libhashish.h"

static const uint32_t murmur_magic = 0x5bd1e995;

#define MURMUR_ROTATE 24

#define MIX(h,k,m) { \
	k *= murmur_magic; \
	k ^= k >> MURMUR_ROTATE; \
	k *= murmur_magic; \
	h *= murmur_magic; h ^= k; }

static inline uint32_t murmur_fini(uint32_t h)
{
	h ^= h >> 13;
	h *= murmur_magic;
	h ^= h >> 15;
	return h;
}

static uint32_t murmur_hash_unaligned(const unsigned char *data, uint32_t len, uint32_t h)
{
	unsigned int align = (unsigned)data & 3;
	uint32_t t = 0, d = 0;
	unsigned int sl, sr = 0;

	switch (align) {
	case 1: t = data[2] << 16;
	case 2: t |= data[1] << 8;
	case 3: t |= data[0];
	}

	t <<= (8 * align);

	data += 4-align;
        len -= 4-align;

	sl = 8 * (4-align);
	if (align < 4)
		sr = 8 * align;

	for (; len >= 4; len-=4, data+=4) {
		uint32_t k;
		d = *(uint32_t *)data;
		t = (t >> sr) | (d << sl);

		k = t;

		MIX(h, k, murmur_magic);

		t = d;
	}

	/* Handle leftover data in temp registers */
	d = 0;
	if (len >= align) {
		uint32_t k;
		switch(align) {
		case 3: d |= data[2] << 16;
		case 2: d |= data[1] << 8;
		case 1: d |= data[0];
		}

		k = (t >> sr) | (d << sl);
		MIX(h,k,murmur_magic);

		data += align;
		len -= align;

		/* Handle tail bytes */
		switch(len) {
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= murmur_magic;
		}
	} else {
		switch (len) {
		case 3: d |= data[2] << 16;
		case 2: d |= data[1] << 8;
		case 1: d |= data[0];
		case 0: h ^= (t >> sr) | (d << sl);
			h *= murmur_magic;
		}
	}
	return murmur_fini(h);
}


static uint32_t murmur_hash2(const void * key, uint32_t len, uint32_t seed)
{
	const unsigned char * data = (const unsigned char *)key;
	uint32_t h = seed ^ len;

	if ((((unsigned)data) & 3) && (len >= 4))
		return murmur_hash_unaligned(data, len, h);

	while (len >= 4) {
		uint32_t k = *(uint32_t *)data;

		MIX(h, k, murmur_magic);

		data += 4;
		len -= 4;
	}

	//----------
	// Handle tail bytes

	switch (len) {
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= murmur_magic;
	}
	return murmur_fini(h);
}

uint32_t lhi_hash_murmur(const uint8_t *key, uint32_t len)
{
	return murmur_hash2(key,len, 0);
}
