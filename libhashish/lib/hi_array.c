/*
** $Id$
**
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

#include <stdlib.h>

#include "privlibhashish.h"

#include "threads.h"

/**
 * hi_get_array return for a given key the correspond data entry
 *
 * @arg hi_handle the hashish handle
 * @arg key a pointer to the key
 * @arg keylen the length of the key in bytes
 * @data the pointer-pointer for the returned data
 * @returns FAILURE or SUCCESS on success and set data pointer
 */
int lhi_get_array(const hi_handle_t *hi_handle, const void *key,
		uint32_t keylen, void **data)
{
	uint32_t bucket, i, already_checked;

	bucket =  hi_handle->hash_func(key, keylen) % hi_handle->table_size;

	already_checked = 0;

	switch (hi_handle->coll_eng) {
		case COLL_ENG_ARRAY:
			lhi_pthread_mutex_lock(hi_handle->mutex_lock);
			for (i = 0; i <
					hi_handle->eng_array.bucket_array_slot_max[bucket]; i++) {

				int diff;

				/* check if we exceed the number of elements in this bucket.
				 * Think about the fact not to run till the end of the
				 * array if we already know that we checked all elements in
				 * this array. This can happen if the array was enlarged in
				 * the beginning and afterwards many elements are removed.
				 * This leads to an sparsely populated array  --HGN */
				if (already_checked >=
						hi_handle->eng_array.bucket_array_slot_size[bucket]) {
					lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
					return HI_ERR_NOKEY;
				}

				/* look if this particular element is an valid one
				 * (or placeholder) */
				if (hi_handle->eng_array.bucket_array[bucket][i].allocation ==
						BA_NOT_ALLOCATED) {
					continue;
				}

				/* now do the trivial key compare */
				diff = hi_handle->key_cmp(key,
							hi_handle->eng_array.bucket_array[bucket][i].key);
				if (diff == 0) {
					*data = (void *) hi_handle->eng_array.bucket_array[bucket][i].data;
					lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
					return SUCCESS;
				}

				++already_checked;

			}
			lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
			return HI_ERR_NOKEY;
			break;
		case COLL_ENG_ARRAY_DYN:
			/* FIXME */
			break;
		case COLL_ENG_ARRAY_DYN_HASH:
			/* FIXME */
			break;
		case COLL_ENG_ARRAY_HASH:
			/* FIXME */
			break;
		default:
			return HI_ERR_INTERNAL;
			break;
	}

	return HI_ERR_NOKEY;
}

/**
 * hi_remove_array remove a complete dataset from the hash set
 *
 * @arg hi_handle the hashish handle
 * @arg key a pointer to the key
 * @arg keylen the length of the key in bytes
 * @data the pointer-pointer for the returned data
 * @returns FAILURE or SUCCESS on success and set data pointer
 */
int lhi_remove_array(hi_handle_t *hi_handle, const void *key,
		uint32_t keylen, void **data)
{
	uint32_t bucket, i, already_checked;
	int ret = SUCCESS;

	already_checked = 0;

	bucket = hi_handle->hash_func(key, keylen) % hi_handle->table_size;

	switch (hi_handle->coll_eng) {
		case COLL_ENG_ARRAY:
			lhi_pthread_mutex_lock(hi_handle->mutex_lock);
			for (i = 0; i <
					hi_handle->eng_array.bucket_array_slot_max[bucket]; i++) {

				int diff;

				/* check if we exceed the number of elements in this bucket.
				 * Think about the fact not to run till the end of the
				 * array if we already know that we checked all elements in
				 * this array. This can happen if the array was enlarged in
				 * the beginning and afterwards many elements are removed.
				 * This leads to an sparsely populated array  --HGN */
				if (already_checked >=
						hi_handle->eng_array.bucket_array_slot_size[bucket]) {
					lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
					return HI_ERR_NOKEY;
				}

				/* look if this particular element is an valid one
				 * (or placeholder) */
				if (hi_handle->eng_array.bucket_array[bucket][i].allocation ==
						BA_NOT_ALLOCATED) {
					continue;
				}

				/* now do the trivial key compare */
				diff = hi_handle->key_cmp(key,
							hi_handle->eng_array.bucket_array[bucket][i].key);
				if (diff == 0) {
					*data = (void *) hi_handle->eng_array.bucket_array[bucket][i].data;

					/* and mark this entry as free */
					hi_handle->eng_array.bucket_array[bucket][i].allocation =
						BA_NOT_ALLOCATED;
					hi_handle->eng_array.bucket_array_slot_size[bucket]--;
					hi_handle->no_objects--;

					lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
					return SUCCESS;
				}

				++already_checked;

			}
			lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
			return HI_ERR_NOKEY;
			break;
		case COLL_ENG_ARRAY_DYN:
			/* FIXME */
			break;
		case COLL_ENG_ARRAY_DYN_HASH:
			/* FIXME */
			break;
		case COLL_ENG_ARRAY_HASH:
			/* FIXME */
			break;
		default:
			ret = HI_ERR_INTERNAL;
			break;
	} /* switch */

	return HI_ERR_NOKEY;
}


int lhi_array_bucket_to_array(const hi_handle_t *hi_handle, size_t bucket, struct lhi_bucket_array *a)
{
	unsigned int len, i, j;
	int ret = HI_ERR_NODATA;

	lhi_pthread_mutex_lock(hi_handle->mutex_lock);
	len = hi_handle->eng_array.bucket_array_slot_size[bucket];
	if (len == 0)
		goto out;

	ret = lhi_bucket_array_alloc(a, len);
	if (ret)
		goto out;

	j = 0;
	for (i = 0; i < len ; i++) {
		/* the array CAN contain spare data buckets, skip it if
		 * we found such bucket */
		if (hi_handle->eng_array.bucket_array[bucket][i].allocation ==
				BA_NOT_ALLOCATED)
			continue;

		a->data[j] = (void *) hi_handle->eng_array.bucket_array[bucket][i].data;
		a->keys[j] = (void *) hi_handle->eng_array.bucket_array[bucket][i].key;
		a->keys_length[i] = hi_handle->eng_array.bucket_array[bucket][i].key_len;
		j++;
	}
 out:
	lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	return ret;
}


/* lhi_insert_array insert a key/data pair into our hashhandle
 *
 * @arg hi_handle the hashish handle
 * @return SUCCESS or a negativ return values in the case of an error
 */
int lhi_insert_array(hi_handle_t *hi_handle, const void *key,
		uint32_t keylen, const void *data)
{
	uint32_t bucket, i;

	bucket = hi_handle->hash_func(key, keylen) % hi_handle->table_size;

	/* check if the key is already in the array
	 * TODO: we should split lhi_get_array to avoid the overhead
	 * but due to the fact that the implementation isn't trivial
	 * and currently in developing this is the actual state */
	if (lhi_get_array(hi_handle, key, keylen, (void **) &data) == SUCCESS)
		return HI_ERR_DUPKEY;



	lhi_pthread_mutex_lock(hi_handle->mutex_lock);

	/* check if the free place is exhausted. If this is
	 * true we must increase the array by a defined factor */
	if (hi_handle->eng_array.bucket_array_slot_size[bucket] >=
			hi_handle->eng_array.bucket_array_slot_max[bucket]) {

		uint32_t old_bucket_size;

		old_bucket_size = hi_handle->eng_array.bucket_array_slot_max[bucket];

		/* double bucket size */
		hi_handle->eng_array.bucket_array_slot_max[bucket] =
			hi_handle->eng_array.bucket_array_slot_max[bucket] << 1;

		hi_handle->eng_array.bucket_array[bucket] = realloc(hi_handle->eng_array.bucket_array[bucket],
				sizeof(hi_bucket_a_obj_t) * hi_handle->eng_array.bucket_array_slot_max[bucket]);
		if (hi_handle->eng_array.bucket_array[bucket] == NULL) {
			lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
			return HI_ERR_SYSTEM;
		}

		/* set up the newly allocated data structures */
		for (i = old_bucket_size; i <
				hi_handle->eng_array.bucket_array_slot_max[bucket]; ++i) {
			hi_handle->eng_array.bucket_array[bucket][i].allocation = BA_NOT_ALLOCATED;
		}
	}

	/* check for the first free elements (BA_NOT_ALLOCATED)
	 * and insert the new one */
	for (i = 0; i < hi_handle->eng_array.bucket_array_slot_max[bucket]; ++i) {
		if (hi_handle->eng_array.bucket_array[bucket][i].allocation == BA_NOT_ALLOCATED) {

			/* add key/data add next free slot */
			hi_handle->eng_array.bucket_array[bucket][i].key = key;
			hi_handle->eng_array.bucket_array[bucket][i].key_len = keylen;
			hi_handle->eng_array.bucket_array[bucket][i].data = data;
			hi_handle->eng_array.bucket_array[bucket][i].allocation = BA_ALLOCATED;

			hi_handle->eng_array.bucket_array_slot_size[bucket]++;
			hi_handle->no_objects++;

			lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
			return SUCCESS;
		}
	}

	/* should never happened */
	lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	return HI_ERR_INTERNAL;
}

int lhi_fini_array(hi_handle_t *hi_handle)
{
	uint32_t i;

	lhi_pthread_mutex_lock(hi_handle->mutex_lock);
	for (i = 0; i < hi_handle->table_size; i++) {
		free(hi_handle->eng_array.bucket_array[i]);
	}
	free(hi_handle->eng_array.bucket_array);
	free(hi_handle->eng_array.bucket_array_slot_size);
	free(hi_handle->eng_array.bucket_array_slot_max);
	lhi_pthread_mutex_unlock(hi_handle->mutex_lock);

	return SUCCESS;
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
