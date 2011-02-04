/*
** $Id: libhashish.c 15 2007-08-23 15:17:56Z hgndgtl $
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

#include "privlibhashish.h"

#include <stdlib.h>
#include <string.h>

#include "threads.h"

const struct hashfunc_map_t lhi_hashfunc_map[] = {
	{HI_HASH_DJB2, "djb2", lhi_hash_djb2},
	{HI_HASH_DUMB1, "dumb1", lhi_hash_dumb1},
	{HI_HASH_ELF, "elf", lhi_hash_elf},
	{HI_HASH_GOULBURN, "goulburn", lhi_hash_goulburn},
	{HI_HASH_HSIEH, "hsieh", lhi_hash_hsieh},
	{HI_HASH_JENKINS2, "jenkins2", lhi_hash_jenkins2},
	{HI_HASH_JENKINS3, "jenkins3", lhi_hash_jenkins3},
	{HI_HASH_PHONG, "phong", lhi_hash_phong},
	{HI_HASH_TOREK, "torek", lhi_hash_torek},
	{HI_HASH_XOR, "xor", lhi_hash_xor},
	{HI_HASH_KORZENDORFER1, "korzendorfer1", lhi_hash_korzendorfer1},
	{HI_HASH_KORZENDORFER2, "korzendorfer2", lhi_hash_korzendorfer2},
	{HI_HASH_SHA1, "sha1", lhi_hash_sha1},
	{HI_HASH_MURMUR, "murmur", lhi_hash_murmur},
	{HI_HASH_CUBE, "cube", lhi_hash_cube},
	{HI_HASH_SKEIN256, "skein256", lhi_hash_skein256}
};


int lhi_create_vanilla_hdnl(hi_handle_t **hi_hndl)
{
	int ret;
	hi_handle_t *hi_handle;

	ret = XMALLOC((void **) &hi_handle, sizeof(hi_handle_t));
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}

	memset(hi_handle, 0, sizeof(hi_handle_t));

	*hi_hndl = hi_handle;

	return SUCCESS;
}

static int lhi_set_sanity_hash2(struct hi_init_set *hi_set)
{
	if (hi_set->hash2_func == NULL)
		return HI_ERR_NODATA;

	if (hi_set->hash_func == hi_set->hash2_func)
		return HI_ERR_NODATA;

	return SUCCESS;
}

static int lhi_set_sanity_array(struct hi_init_set *hi_set)
{
	if (hi_set->coll_eng_array_size == 0)
		return HI_ERR_NODATA;

	return SUCCESS;
}

static int lhi_set_sanity_check(struct hi_init_set *hi_set)
{
	int ret;

	if (hi_set->table_size == 0)
		return HI_ERR_NODATA;

	if (hi_set->key_cmp == NULL)
		return HI_ERR_NODATA;

	if (hi_set->hash_func == NULL)
		return HI_ERR_NODATA;

	switch (hi_set->coll_eng) {

		case COLL_ENG_LIST:
			/* nothing to check here - all tests are covered
			 * in the above statements
			 */
			break;
		case COLL_ENG_LIST_HASH:
			/* Test if a second hash function is given
			 * and this function isn't the same to the
			 * primary hash function.
			 */
			ret = lhi_set_sanity_hash2(hi_set);
			if (ret != SUCCESS)
				return ret;
			break;
		case COLL_ENG_LIST_MTF:
			break;
		case COLL_ENG_LIST_MTF_HASH:
			/* Test if a second hash function is given
			 * and this function isn't the same to the
			 * primary hash function.
			 */
			ret = lhi_set_sanity_hash2(hi_set);
			if (ret != SUCCESS)
				return ret;
			break;
		case COLL_ENG_ARRAY:
			ret = lhi_set_sanity_array(hi_set);
			if (ret != SUCCESS)
				return ret;
			break;
		case COLL_ENG_ARRAY_HASH:
			break;
		case COLL_ENG_ARRAY_DYN:
			break;
		case COLL_ENG_ARRAY_DYN_HASH:
			/* Test if a second hash function is given
			 * and this function isn't the same to the
			 * primary hash function.
			 */
			ret = lhi_set_sanity_hash2(hi_set);
			if (ret != SUCCESS)
				return ret;
			break;
		case COLL_ENG_RBTREE:
			break;
		default:
			return HI_ERR_INTERNAL;
			break;

	}

	return SUCCESS;
}

static int lhi_transform_set_2_hndl(hi_handle_t *hi_hndl,
		struct hi_init_set *hi_set)
{
	int ret;

	/* hi_init_set full sanity checks (all
	 * information is available */
	ret = lhi_set_sanity_check(hi_set);
	if (ret != SUCCESS)
		return ret;

	/* NOTE: it is also possible to integrate the hi_set within
	 * hi_hndl and memcpy the data. BUT maybe in some further revisions
	 * we must/should transform values from user perspective to a internal
	 * representation this additional step is more generic.
	 */

	hi_hndl->table_size           = hi_set->table_size;
	hi_hndl->hash_func            = hi_set->hash_func;
	hi_hndl->hash2_func           = hi_set->hash2_func;
	hi_hndl->key_cmp              = hi_set->key_cmp;
	hi_hndl->coll_eng             = hi_set->coll_eng;
	hi_hndl->rehash_auto          = hi_set->rehash_auto;
	hi_hndl->rehash_threshold     = hi_set->rehash_threshold;
	hi_hndl->coll_eng_array_size  = hi_set->coll_eng_array_size;

	return SUCCESS;
}

void lhi_transform_hndl_2_hndl(hi_handle_t *hi_hndl_src,
		hi_handle_t *hi_hndl_dst)
{

	hi_hndl_dst->table_size          = hi_hndl_src->table_size;
	hi_hndl_dst->hash_func           = hi_hndl_src->hash_func;
	hi_hndl_dst->hash2_func          = hi_hndl_src->hash2_func;
	hi_hndl_dst->key_cmp             = hi_hndl_src->key_cmp;
	hi_hndl_dst->coll_eng            = hi_hndl_src->coll_eng;
	hi_hndl_dst->rehash_auto         = hi_hndl_src->rehash_auto;
	hi_hndl_dst->rehash_threshold    = hi_hndl_src->rehash_threshold;
	hi_hndl_dst->coll_eng_array_size = hi_hndl_src->coll_eng_array_size;

}


static int lhi_create_eng_list(hi_handle_t *hi_hndl)
{
	uint32_t i; int ret;

	/* This is the intrinsic table which contains
	 * the pointers to the list-heads.
	 */
	ret = XMALLOC((void **) &hi_hndl->eng_list.bucket_table,
			hi_hndl->table_size * sizeof(void *));
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}

	/* initialize bucket list */
	for (i = 0; i < hi_hndl->table_size; i++) {
		hi_hndl->eng_list.bucket_table[i] = NULL;
		hi_hndl->bucket_size[i] = 0;
	}

	return SUCCESS;
}


static int lhi_create_eng_rbtree(hi_handle_t *hi_hndl)
{
	uint32_t i;

	hi_hndl->eng_rbtree.trees = calloc(sizeof(struct __hi_rb_tree), hi_hndl->table_size);
	if (!hi_hndl->eng_rbtree.trees)
		return HI_ERR_SYSTEM;

	lhi_pthread_mutex_lock(hi_hndl->mutex_lock);

	for (i = 0; i < hi_hndl->table_size; i++) {
		hi_hndl->eng_rbtree.trees[i].root.rb_node = NULL;
		hi_hndl->eng_rbtree.trees[i].rwlock = NULL;
		hi_hndl->bucket_size[i] = 0;
		if (lhi_pthread_rwlock_init(&hi_hndl->eng_rbtree.trees[i].rwlock, NULL))
			goto out_err;
	}
	lhi_pthread_mutex_unlock(hi_hndl->mutex_lock);
	return SUCCESS;
 out_err:
	while (i--)
		lhi_pthread_rwlock_destroy(hi_hndl->eng_rbtree.trees[i].rwlock);
	return HI_ERR_SYSTEM;
}


static int lhi_create_eng_array(hi_handle_t *hi_hndl)
{
	int ret; uint32_t i, j;

	/* the number of elements in this bucket */
	ret = XMALLOC((void **) &hi_hndl->eng_array.bucket_array_slot_size,
			sizeof(unsigned int) * hi_hndl->table_size);
	if (ret != 0)
		return HI_ERR_SYSTEM;

	/* the maximum number of elements for this bucket (realloc necessary
	 * if the space is insufficient */
	ret = XMALLOC((void **) &hi_hndl->eng_array.bucket_array_slot_max,
			sizeof(unsigned int) * hi_hndl->table_size);
	if (ret != 0)
		return HI_ERR_SYSTEM;

	/* the pointer for the several buckets */
	ret = XMALLOC((void **) &hi_hndl->eng_array.bucket_array,
			(sizeof(hi_bucket_a_obj_t *) * hi_hndl->table_size));
	if (ret != 0)
		return HI_ERR_SYSTEM;

	/* last but not least: the buckets */
	for (i = 0; i < hi_hndl->table_size; i++) {
		/* align array on 16 byte boundaries */
		ret = xalloc_align((void **) &hi_hndl->eng_array.bucket_array[i],
				LHI_DEFAULT_MEMORY_ALIGN,
				(sizeof(hi_bucket_a_obj_t) * hi_hndl->coll_eng_array_size));
		if (ret != 0)
			return HI_ERR_SYSTEM;

		/* a bucket_array[i] handles key, data and one additional field:
		 * the allocation of the current element within the bucket. This
		 * is essential for the bucket structure cause through due remove
		 * method wholes can arise. One answer is to reorder the whole
		 * structure or (like this implementation does), flag the element
		 * as BA_NOT_ALLOCATED (or BA_ALLOCATED on the other hand)  --HGN */

		for (j = 0; j < hi_hndl->coll_eng_array_size; ++j)
			hi_hndl->eng_array.bucket_array[i][j].allocation = BA_NOT_ALLOCATED;

		/* some status bookkeeping */
		hi_hndl->eng_array.bucket_array_slot_size[i] = 0;
		hi_hndl->eng_array.bucket_array_slot_max[i]  = hi_hndl->coll_eng_array_size;
	}

	return SUCCESS;
}

/**
 * This is the default initialize function. It takes HI_HASH_DEFAULT as the
 * default hash function and set compare function for strings - so use it only
 * for strings
 *
 * @arg hi_hndl	this become out new hashish handle
 * @arg buckets	hash bucket size
 * @returns negativ error value or zero on success
 */
int hi_create(hi_handle_t **hi_hndl, struct hi_init_set *hi_set)
{
	int ret;
	hi_handle_t *hi_handle;

	ret = lhi_create_vanilla_hdnl(&hi_handle);
	if (ret != SUCCESS)
		return ret;

	/* Check values in hi_set and transform user
	 * representation to internal representation */
	ret = lhi_transform_set_2_hndl(hi_handle, hi_set);
	if (ret != SUCCESS)
		return ret;

	/* Allocate memory fot accounting the number of
	 * elements within every bucket in the table.  */
	ret = XMALLOC((void **) &hi_handle->bucket_size,
			hi_handle->table_size * sizeof(hi_handle->bucket_size));
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}

	/* 0 objects in the list at start-up */
	hi_handle->no_objects = 0;

	/* Initiate mutex lock if build with thread
	 * support. */
	hi_handle->mutex_lock = NULL;
	ret = lhi_pthread_mutex_init(&hi_handle->mutex_lock, NULL);
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}

	/* Create internal data structure for
	 * list, array or rbtree */
	switch (hi_handle->coll_eng) {

		case COLL_ENG_LIST:
		case COLL_ENG_LIST_HASH:
		case COLL_ENG_LIST_MTF:
		case COLL_ENG_LIST_MTF_HASH:
			ret = lhi_create_eng_list(hi_handle);
			if (ret != SUCCESS)
				return ret;
			break;
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			ret = lhi_create_eng_array(hi_handle);
			if (ret != SUCCESS)
				return ret;
			break;
		case COLL_ENG_RBTREE:
			ret = lhi_create_eng_rbtree(hi_handle);
			if (ret != SUCCESS)
				return ret;
			break;
		default:
			return HI_ERR_INTERNAL;
			break;
	}

	*hi_hndl = hi_handle;

	return SUCCESS;
}

/**
 *
 * @arg hi_hndl	this become out new hashish handle
 * @returns negativ error value or zero on success
 */

int hi_rehash(hi_handle_t *hi_hndl, uint32_t new_table_size)
{
	int ret, auto_rehash;
	void *key, *data;
	uint32_t keylen;
	hi_handle_t *hi_handle;
	hi_iterator_t *iterator;

	/* create a clean handle for the new structure */
	ret = lhi_create_vanilla_hdnl(&hi_handle);
	if (ret != SUCCESS)
		return ret;

	/* we take over the original settings done
	 * by the user taken at hi_create() time */
	lhi_transform_hndl_2_hndl(hi_hndl, hi_handle);

	hi_handle->table_size = new_table_size;

	/* Allocate memory fot accounting the number of
	 * elements within every bucket in the table.  */
	ret = XMALLOC((void **) &hi_handle->bucket_size,
			hi_handle->table_size * sizeof(*hi_handle->bucket_size));
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}

	/* 0 objects in the list at start-up */
	hi_handle->no_objects = 0;

	/* Initiate mutex lock if build with thread
	 * support. */
	hi_handle->mutex_lock = NULL;
	ret = lhi_pthread_mutex_init(&hi_handle->mutex_lock, NULL);
	if (ret != 0) {
		return HI_ERR_SYSTEM;
	}

	/* Create internal data structure for
	 * list, array or rbtree */
	switch (hi_handle->coll_eng) {

		case COLL_ENG_LIST:
		case COLL_ENG_LIST_HASH:
		case COLL_ENG_LIST_MTF:
		case COLL_ENG_LIST_MTF_HASH:
			ret = lhi_create_eng_list(hi_handle);
			if (ret != SUCCESS)
				return ret;


			break;
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			ret = lhi_create_eng_array(hi_handle);
			if (ret != SUCCESS)
				return ret;


			break;
		case COLL_ENG_RBTREE:
			ret = lhi_create_eng_rbtree(hi_handle);
			if (ret != SUCCESS)
				return ret;


			break;
		default:
			return HI_ERR_INTERNAL;
			break;
	}

	ret = hi_iterator_create(hi_hndl, &iterator);
	if (ret != SUCCESS)
		return ret;

	auto_rehash = hi_handle->rehash_auto;
	hi_handle->rehash_auto = 0;
	while ((ret = hi_iterator_getnext(iterator, &data, &key, &keylen)) ==
			SUCCESS) {
		ret = hi_insert(hi_handle, key, keylen, data);
		if (ret != SUCCESS) {
			hi_iterator_fini(iterator);
			hi_fini(hi_handle);

			hi_handle->rehash_auto = auto_rehash;
			return ret;
		}
	}
	hi_handle->rehash_auto = auto_rehash;
	/* verify that no error occured during iterator run */
	if (ret != HI_ERR_NODATA) {
		hi_iterator_fini(iterator);
		return ret;
	}

	hi_iterator_fini(iterator);

	/* free old hashish handle */
	lhi_fini_internal(hi_hndl);

	memcpy(hi_hndl, hi_handle, sizeof(*hi_hndl));

	return SUCCESS;
}


/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
