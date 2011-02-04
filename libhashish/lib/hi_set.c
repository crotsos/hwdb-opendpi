/*
** Copyright (C) 2006 - Hagen Paul Pfeifer <hagen@jauu.net>
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

#include <string.h>

#include "privlibhashish.h"

/**
 * Initialize the hi_init_set structure. This function
 * should be called for every new hi_init_set structure.
 *
 * @arg hi_set	the initial set struct
 */
void hi_set_zero(struct hi_init_set *hi_set)
{
	memset(hi_set, 0, sizeof(struct hi_init_set));
	hi_set->rehash_threshold = 0.75;
}

/**
 * Set the bucket size for an spezific hi_init_set and
 * therefore the structure at all.
 *
 * @arg hi_set	the initial structure set
 * @arg size	the size of the hash
 * @returns	negative error value or zero on success
 */
int hi_set_bucket_size(struct hi_init_set *hi_set, uint32_t size)
{
	if (size == 0)
		return HI_ERR_RANGE;

	hi_set->table_size = size;

	return SUCCESS;
}

/**
 * Set the initial array size per hash bucket. Normally you should
 * select the expected average bucket capacity plus 1 (which should be not more
 * then 1 - we are a hash data structure at least). The additional element is
 * a backing buffer - to handle collisions. The overhead is marginal if the
 * hash table size isn't that much. If you select a to small array size then
 * realloc() must be called to often.
 *
 * @arg hi_set  the initial structure set
 * @arg size    the size of the array size per bucket
 * @returns a negative error value or zero on success
 */
int hi_set_coll_eng_array_size(struct hi_init_set *hi_set, uint32_t size)
{
	if (size == 0)
		return HI_ERR_RANGE;

	hi_set->coll_eng_array_size = size;

	return SUCCESS;
}

int hi_set_hash_alg(struct hi_init_set *hi_set, enum hash_alg hash_alg)
{
	int i;

	/* loop until we found the right one */
	for (i = 0; i <= HI_HASH_MAX; ++i) {
		if (hash_alg == lhi_hashfunc_map[i].no) {
			hi_set->hash_func = lhi_hashfunc_map[i].hashfunc;
			return SUCCESS;
		}
	}
	return HI_ERR_NOFUNC;
}

int hi_set_hash_func(struct hi_init_set *hi_set,
		uint32_t (*hash_func)(const uint8_t*, uint32_t))
{
	if (hash_func == NULL)
		return HI_ERR_NODATA;

	hi_set->hash_func = hash_func;

	return SUCCESS;
}

int hi_set_hash2_alg(struct hi_init_set *hi_set, enum hash_alg hash_alg)
{
	int i;

	/* loop until we found the right one */
	for (i = 0; i <= HI_HASH_MAX; ++i) {
		if (hash_alg == lhi_hashfunc_map[i].no) {
			hi_set->hash2_func = lhi_hashfunc_map[i].hashfunc;
			return SUCCESS;
		}
	}
	return HI_ERR_NOFUNC;

}

int hi_set_hash2_func(struct hi_init_set *hi_set,
		uint32_t (*hash_func)(const uint8_t*, uint32_t))
{
	if (hash_func == NULL)
		return HI_ERR_NODATA;

	hi_set->hash2_func = hash_func;

	return SUCCESS;
}

int hi_set_coll_eng(struct hi_init_set *hi_set, enum coll_eng coll_eng)
{
	if (coll_eng > COLL_ENG_MAX)
		return HI_ERR_NOFUNC;

	hi_set->coll_eng = coll_eng;

	return SUCCESS;
}

void hi_set_rehash_auto(struct hi_init_set *hi_set, int choice)
{
	hi_set->rehash_auto = choice;
}

int hi_set_key_cmp_func(struct hi_init_set *hi_set,
		int (*cmp)(const uint8_t *, const uint8_t *))
{
	if (cmp == NULL)
		return HI_ERR_NODATA;

	hi_set->key_cmp = cmp;

	return SUCCESS;
}



/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
