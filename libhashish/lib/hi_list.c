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

#include <stdlib.h>

#include "threads.h"
#include "privlibhashish.h"

int lhi_list_bucket_to_array(const hi_handle_t *hi_handle, size_t bucket, struct lhi_bucket_array *a)
{
	size_t max, i = 0;
	int ret = HI_ERR_NODATA;

	if (hi_handle->table_size < bucket)
		return HI_ERR_RANGE;

	lhi_pthread_mutex_lock(hi_handle->mutex_lock);
	max = hi_handle->bucket_size[bucket];
	if (!max)
		goto out_err;
	ret = lhi_bucket_array_alloc(a, max);
	if (ret)
		goto out_err;

	switch (hi_handle->coll_eng) {
	case COLL_ENG_LIST:
	case COLL_ENG_LIST_MTF: {
		hi_bucket_obj_t *b_obj = hi_handle->eng_list.bucket_table[bucket];
		for (; b_obj; b_obj = b_obj->next) {
			a->data[i] = (void*) b_obj->data;
			a->keys[i] = (void*) b_obj->key;
			a->keys_length[i] = b_obj->key_len;
			i++;
		}
	break;
	}
	case COLL_ENG_LIST_HASH:
	case COLL_ENG_LIST_MTF_HASH: {
		hi_bucket_hl_obj_t *b_obj = hi_handle->eng_list.bucket_table_hl[bucket];
		for (; b_obj; b_obj = b_obj->next) {
			a->data[i] = (void*) b_obj->data;
			a->keys[i] = (void*) b_obj->key;
			a->keys_length[i] = b_obj->key_len;
			i++;
		}
	break;
	}
	default:
		ret = HI_ERR_INTERNAL;
		goto out_err;
	}
	lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	return 0;
 out_err:
	lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	return ret;
}

/* lhi_lookup_list search for a given key and return SUCCESS
 * when found in the hash and FAILURE if not found.
 *
 * @arg hi_handle the hashish handle
 * @arg key the pointer to the key
 * @arg keylen the len of the key in bytes
 * @return SUCCESS if found or FAILURE when not found
 */

int lhi_lookup_list(hi_handle_t *hi_handle,
		const void *key, uint32_t keylen)
{
	uint32_t bucket;

	bucket =  hi_handle->hash_func(key, keylen) % hi_handle->table_size;

	switch (hi_handle->coll_eng) {
	case COLL_ENG_LIST:
	case COLL_ENG_LIST_MTF: {
		hi_bucket_obj_t *b_obj = hi_handle->eng_list.bucket_table[bucket];

		for (; b_obj; b_obj = b_obj->next) {
			if (hi_handle->key_cmp(key, b_obj->key) == 0)
				return SUCCESS;
		}
	}
	return HI_ERR_NOKEY;
	case COLL_ENG_LIST_HASH:
	case COLL_ENG_LIST_MTF_HASH: {
		uint32_t key_hash = hi_handle->hash2_func(key, keylen);
		hi_bucket_hl_obj_t *b_obj = hi_handle->eng_list.bucket_table_hl[bucket];

		for (; b_obj; b_obj = b_obj->next) {
			if (key_hash == b_obj->key_hash &&
				hi_handle->key_cmp(key, b_obj->key) == 0)
					return SUCCESS;
		}
	}
	return HI_ERR_NOKEY;
	}
	return HI_ERR_INTERNAL;
}


/**
 * hi_remove remove a complete dataset completly from the hash set
 *
 * @arg hi_handle the hashish handle
 * @arg key a pointer to the key
 * @arg keylen the length of the key in bytes
 * @data the pointer-pointer for the returned data
 * @returns FAILURE or SUCCESS on success and set data pointer
 */
int lhi_remove_list(hi_handle_t *hi_handle, const void *key,
		uint32_t keylen, void **data)
{
	uint32_t bucket;
	int ret = SUCCESS;

	lhi_pthread_mutex_lock(hi_handle->mutex_lock);

	bucket = hi_handle->hash_func(key, keylen) % hi_handle->table_size;

	switch (hi_handle->coll_eng) {
	case COLL_ENG_LIST:
	case COLL_ENG_LIST_MTF: {
		hi_bucket_obj_t *p, *b_obj = hi_handle->eng_list.bucket_table[bucket];

		for (p = b_obj; b_obj ; b_obj = b_obj->next) {
			if (hi_handle->key_cmp(key, b_obj->key) == 0) {
				*data = (void *) b_obj->data;
				if (p == b_obj)
					hi_handle->eng_list.bucket_table[bucket] = p->next;
				else
					p->next = b_obj->next;
				free(b_obj);
				goto out_found;
			}
			p = b_obj;
		}
		ret = HI_ERR_NOKEY;
		break;
	}
	case COLL_ENG_LIST_HASH:
	case COLL_ENG_LIST_MTF_HASH: {
		hi_bucket_hl_obj_t *p, *b_obj = hi_handle->eng_list.bucket_table_hl[bucket];
		uint32_t key_hash = hi_handle->hash2_func(key, keylen);

		for (p = b_obj; b_obj ; b_obj = b_obj->next) {
			if (key_hash == b_obj->key_hash &&
				hi_handle->key_cmp(key, b_obj->key) == 0)
			{
				*data = (void *) b_obj->data;
				if (p == b_obj)
					hi_handle->eng_list.bucket_table_hl[bucket] = p->next;
				else
					p->next = b_obj->next;
				free(b_obj);
				goto out_found;
			}
			p = b_obj;
		}
		ret = HI_ERR_NOKEY;
		break;
	}
	default:
		ret = HI_ERR_INTERNAL;
		break;
	} /* switch */

 out:
	lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	return ret;
 out_found:
	--hi_handle->bucket_size[bucket];
	--hi_handle->no_objects;
	goto out;
}

/**
 * hi_get_list return for a given key the correspond data entry
 *
 * @arg hi_handle the hashish handle
 * @arg key a pointer to the key
 * @arg keylen the length of the key in bytes
 * @data the pointer-pointer for the returned data
 * @returns FAILURE or SUCCESS on success and set data pointer
 */
int lhi_get_list(const hi_handle_t *hi_handle, const void *key,
		uint32_t keylen, void **data)
{
	uint32_t bucket;

	bucket =  hi_handle->hash_func(key, keylen) % hi_handle->table_size;
	switch (hi_handle->coll_eng) {
	case COLL_ENG_LIST: {
		lhi_pthread_mutex_lock(hi_handle->mutex_lock);
		hi_bucket_obj_t *b_obj = hi_handle->eng_list.bucket_table[bucket];

		for (; b_obj ; b_obj = b_obj->next) {
			if (hi_handle->key_cmp(key, b_obj->key) == 0) {
				*data = (void *) b_obj->data;
				lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
				return SUCCESS;
			}
		}
		lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	}
	return HI_ERR_NOKEY;
            /* CHAINING_LIST_MTF is nearly equal to the CHAINING_LIST
             * strategy except to the hi_get routine:
             * This strategy favors often used elements by doing a swapping
             * of elements (key and data) to the beginning of the list.
             * Therefore if the searched elements are underlie no normal
             * distribution this strategy may have an advantage. The
             * disadvantage of the algorithm is the swap routine - of
             * course.
             */
	case COLL_ENG_LIST_MTF: {
			/* Key and data are pointers - therefore we store
			 * the pointer to the first set, search the right
			 * set and reorder the set.
			 */
		hi_bucket_obj_t *p, *b_obj = hi_handle->eng_list.bucket_table[bucket];

		for (p = b_obj; b_obj != NULL; b_obj = b_obj->next) {
			if (hi_handle->key_cmp(key, b_obj->key) == 0) {
				*data = (void *) b_obj->data;
				if (p != b_obj) { /* put this one at the front */
					p->next = b_obj->next;
					b_obj->next = hi_handle->eng_list.bucket_table[bucket];
					hi_handle->eng_list.bucket_table[bucket] = b_obj;
				}
				lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
				return SUCCESS;
			}
			p = b_obj;
		}
		lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
		}
		return HI_ERR_NOKEY;
	case COLL_ENG_LIST_HASH: {
		hi_bucket_hl_obj_t *b_obj;
		lhi_pthread_mutex_lock(hi_handle->mutex_lock);
		b_obj = hi_handle->eng_list.bucket_table_hl[bucket];
		uint32_t key_hash = hi_handle->hash2_func(key, keylen);

		for (;b_obj != NULL; b_obj = b_obj->next) {
			if (key_hash == b_obj->key_hash &&
				hi_handle->key_cmp(key, b_obj->key) == 0)
			{
				*data = (void *) b_obj->data;
				lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
				return SUCCESS;
			}
		}
		lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	}
	return HI_ERR_NOKEY;
	case COLL_ENG_LIST_MTF_HASH: {
		hi_bucket_hl_obj_t *p, *b_obj;
		lhi_pthread_mutex_lock(hi_handle->mutex_lock);
		b_obj = hi_handle->eng_list.bucket_table_hl[bucket];
		uint32_t key_hash = hi_handle->hash2_func(key, keylen);

		for (p = b_obj; b_obj != NULL; b_obj = b_obj->next) {
			if (key_hash != b_obj->key_hash &&
				hi_handle->key_cmp(key, b_obj->key) == 0)
			{
				*data = (void *) b_obj->data;
				if (p != b_obj) { /* put this one at the front */
					p->next = b_obj->next;
					b_obj->next = hi_handle->eng_list.bucket_table_hl[bucket];
					hi_handle->eng_list.bucket_table_hl[bucket] = b_obj;
				}
				lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
				return SUCCESS;
			}
			p = b_obj;
		}
		lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	}
	return HI_ERR_NOKEY;
	}
	return HI_ERR_INTERNAL;
}

/* lhi_insert_list insert a key/data pair into our hashhandle
 *
 * @arg hi_handle the hashish handle
 * @return SUCCESS or a negativ return values in the case of an error
 */
int lhi_insert_list(hi_handle_t *hi_handle, const void *key,
		uint32_t keylen, const void *data)
{
	hi_bucket_hl_obj_t *obj;
	int ret = HI_ERR_SYSTEM;
	uint32_t bucket;

	lhi_pthread_mutex_lock(hi_handle->mutex_lock);

	bucket = hi_handle->hash_func(key, keylen) % hi_handle->table_size;
	ret = XMALLOC((void **) &obj, sizeof(*obj));
	if (ret != 0)
		goto out;

	obj->key = key;
	obj->key_len = keylen;
	obj->data = data;

	if (hi_handle->eng_list.bucket_table[bucket])
		obj->next = hi_handle->eng_list.bucket_table_hl[bucket];
	else
		obj->next = NULL;

	hi_handle->eng_list.bucket_table_hl[bucket] = obj;

	hi_handle->bucket_size[bucket]++;
	hi_handle->no_objects++;
	ret = SUCCESS;
 out:
	lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	return ret;
}


/* lhi_fini_list delete a complete hashish handle. This function is destroy
 * list specific data. The whole funtion is protected by an global
 * lock.
 *
 * @arg hi_handle the hashish handle
 * @return SUCCESS or a negativ return values in the case of an error
 */
int lhi_fini_list(hi_handle_t *hi_handle)
{
	uint32_t i;

	for (i = 0; i < hi_handle->table_size; i++) {
		hi_bucket_obj_t *b_obj = hi_handle->eng_list.bucket_table[i];
		/* iterate over list and remove all entries and free hi_bucket_obj_t */
		while (b_obj) {
			hi_bucket_obj_t *v = b_obj;
			b_obj = v->next;
			free(v);
		}
	}
	free(hi_handle->eng_list.bucket_table);
	return SUCCESS;
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
