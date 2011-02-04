/*
** Copyright (C) 2008 - Hagen Paul Pfeifer <hagen@jauu.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "privlibhashish.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

struct {
	uint32_t (*hash)(const uint8_t *key, uint32_t len);
} lhi_hashmap[] = {
	{ lhi_hash_hsieh },
	{ lhi_hash_jenkins3 },
	{ lhi_hash_goulburn },
	{ lhi_hash_phong },
	{ lhi_hash_torek },
	{ lhi_hash_xor },
	{ lhi_hash_sdbm }
};


/**
 * Return the current false positive probability for
 * the filter: (1-e^(-kn/m))^k
 * Search the web for a more detailed explanation.
 *
 * @arg bh hashish handle
 * @returns the current false positive probability
 */
double hi_bloom_current_false_positiv_probability(hi_bloom_handle_t *bh)
{
	return pow((1 - pow(M_E, -((double)bh->k * bh->n / bh->m))), (double)bh->k);
}


/**
 * hi_bloom_false_positiv_probability returns the false
 * positive probability for a given set of arguments:
 * (1-e^(-kn/m))^k
 *
 * @arg m is the bitvector size (e.g. 256)
 * @arg n are the amount of inserted elements
 * @arg k is the number of utilized hash functions
 * @returns the current false positive probability
 */
double hi_bloom_false_positiv_probability(uint32_t m, uint32_t n, uint32_t k)
{
	return pow((1 - pow(M_E, -((double)k * n / m))), (double)k);
}


/**
 * hi_bloom_filter_add adds an key into the bloom
 * filter and set all approximate bits for hi_bloom_handle
 * bh. 
 *
 * @arg bh hashish handle
 * @arg key is the key to add into the filter
 * @arg len are the length of the key to calculate (to hash)
 * @returns negative error value or zero when not found and one when found
 */
void hi_bloom_filter_add(hi_bloom_handle_t *bh, uint8_t *key, uint32_t len)
{
	uint32_t map_offset, bit, bit_mask, hkey, iter;

	/* track number of elements */
	bh->n++;

	/* TODO: could be unrolled via switch case and fall through ...
	 * but I am to lazy now ... --HGN */
	for (iter = 0; iter < bh->k; ++iter) {
		hkey = lhi_hashmap[iter].hash(key, len);
		bit  = hkey % bh->m;
		map_offset = floor(bit / 8);
		bit_mask = 1 << (bit & 7);
		if (bh->filter_map[map_offset] & bit_mask)
			bh->bit_collision++;
		bh->filter_map[map_offset] |= bit_mask;
	}
}


/**
 * hi_bloom_filter_check check if a certain key is already
 * in the filter. Remember how bloom filter works - there
 * is no real deterministic characteristic that a certain
 * key was already added to the filter (see
 * hi_bloom_current_false_positiv_probability() for the current
 * probability.
 *
 * @arg bh hashish handle
 * @arg key is the key to add into the filter
 * @arg len is the length of the key to hash
 * @returns negative error value or zero when not found and one when found
 */
int hi_bloom_filter_check(hi_bloom_handle_t *bh, void *gkey, uint32_t len)
{
	uint32_t map_offset, bit, bit_mask, hkey, iter;

	uint8_t *key = (uint8_t *)gkey;

	for (iter = 0; iter < bh->k; ++iter) {
		hkey = lhi_hashmap[iter].hash(key, len);
		bit  = hkey % bh->m;
		map_offset = floor(bit / 8);
		bit_mask = 1 << (bit & 7);
		if (!(bh->filter_map[map_offset] & bit_mask))
			return 0; /* bit not set */
	}

	/* match! */
	return 1;
}

/**
 * hi_bloom_filter_add_str is a simple wrapper around
 * hi_bloom_filter_add to handle string keys more smoothly
 *
 * @arg bh hashish handle
 * @arg key is the key to add into the filter
 * @returns negative error value or zero when not found and one when found
 */
void hi_bloom_filter_add_str(hi_bloom_handle_t *a, const char *b)
{
	hi_bloom_filter_add(a, (uint8_t *) b, strlen(b));
}


/**
 * hi_bloom_filter_check_str is wrapper for hi_bloom_filter_check
 * for strings. The length arguments is substituted through strlen()
 * and the argument types are string conform (e.g. const char *).
 * See hi_bloom_filter_check() for an in deep detail of this function
 * or read the source code. ;-)
 *
 * @arg bh hashish handle
 * @arg key is the key to add into the filter
 * @returns negative error value or zero when not found and one when found
 */
int hi_bloom_filter_check_str(hi_bloom_handle_t *a, const char *b)
{
	return hi_bloom_filter_check(a, (void *) b, strlen(b));
}

/**
 * hi_bloom_bit_get returns if an specified bit
 * in the map is set
 *
 * @arg bh hashish handle
 * @arg bit	to check (start with bit 0 - of course)
 * @returns negative error value or zero when not found and one when found
 */
int hi_bloom_bit_get(hi_bloom_handle_t *bh, uint32_t bit)
{
	uint32_t byte_offset, bit_offset;

	if (!bh)
		return HI_ERR_NODATA;

	if (bit > bh->m)
		return HI_ERR_RANGE;

	byte_offset = (uint32_t)(floor((double)bit / 8));
	bit_offset = (uint32_t)(bit % 8);

	return (!!(bh->filter_map[byte_offset] & (1 << bit_offset)));

}

/**
 * This is for analysis of bloom filter and print
 * out the textural representation of the bit vector
 * in hexadecimal format.
 *
 * @arg bh the hashish handle
 * @returns negativ error value or zero on success
 */
int hi_bloom_print_hex_map(hi_bloom_handle_t *bh)
{
	/* TODO: this function should be replaced
	 * by an more parseable form. The main puspose
	 * of this function is a simple wrapper around the
	 * internal representation of the bitvector -a public accessor.
	 *
	 * A more clean design is a malloc/snprintf/return vector or
	 * simple a malloc/memcpy/ return bitvector
	 *
	 *	--HGN
	 */
	uint32_t byte_offset = 0;

	if (!bh)
		return HI_ERR_NODATA;

	for (byte_offset = 0; byte_offset < bh->m / 8; ++byte_offset) {
		fprintf(stderr, "0x%X ", bh->filter_map[byte_offset]);
	}


	return SUCCESS;
}

/**
 * This is one initialize function for bloom filter.
 * This function must be called to initialize the bloom filter
 *
 * @arg bh	this become out new hashish handle
 * @arg m	hash bucket size
 * @arg k   number of hash algorithms to use
 * @returns negativ error value or zero on success
 */
int hi_bloom_init_mk(hi_bloom_handle_t **bh, uint32_t m, uint32_t k)
{
	int ret;
	hi_bloom_handle_t *nbh;

	if (m % 8 != 0 && m > 0) /* bit size must conform to byte boundaries */
		return HI_ERR_RANGE;

	if (ARRAY_SIZE(lhi_hashmap) < k)
		return HI_ERR_RANGE;

	ret = XMALLOC((void **) &nbh, sizeof(hi_bloom_handle_t));
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}
	memset(nbh, 0, sizeof(hi_bloom_handle_t));

	nbh->k = k;

	/* initialize filter_map */
	ret = XMALLOC((void **) &nbh->filter_map, m / 8);
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}
	memset(nbh->filter_map, 0, m / 8);

	nbh->m = m;

	*bh = nbh;

	return SUCCESS;
}



/**
 * This is finilizer function for bloom filter.
 * This function must be called to free the internal
 * memory used by bloom filter.
 *
 * @arg bh the hashish handle
 */
void hi_fini_bloom_filter(hi_bloom_handle_t *bh)
{
	free(bh->filter_map);
	free(bh);
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
