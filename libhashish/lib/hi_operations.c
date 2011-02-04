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

#include "privlibhashish.h"

#include "threads.h"

/**
 * hi_lookup search for a given key and return SUCCESS
 * when found in the hash and FAILURE if not found.
 *
 * @arg hi_handle the hashish handle
 * @arg key the pointer to the key
 * @arg keylen the len of the key in bytes
 * @return SUCCESS if found or FAILURE when not found
 */
static int hi_lookup(hi_handle_t *hi_handle, const void *key, uint32_t keylen)
{
	int ret;

	switch (hi_handle->coll_eng) {
	case COLL_ENG_LIST:
	case COLL_ENG_LIST_HASH:
	case COLL_ENG_LIST_MTF:
	case COLL_ENG_LIST_MTF_HASH:
		lhi_pthread_mutex_lock(hi_handle->mutex_lock);
		ret = lhi_lookup_list(hi_handle, key, keylen);
		lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
		return ret;
	case COLL_ENG_ARRAY:
	case COLL_ENG_ARRAY_HASH:
	case COLL_ENG_ARRAY_DYN:
	case COLL_ENG_ARRAY_DYN_HASH:
		return FAILURE;
	case COLL_ENG_RBTREE: /* rbtree insert handles dupkey case */
		return FAILURE;
	case __COLL_ENG_MAX: /* avoid 'warning: enumeration value '__COLL_ENG_MAX' not handled in switch' */
		break;
	}
	return HI_ERR_INTERNAL;
}

/**
 * hi_get return for a given key the correspond data entry
 *
 * @arg hi_handle the hashish handle
 * @arg key a pointer to the key
 * @arg keylen the length of the key in bytes
 * @data the pointer-pointer for the returned data
 * @returns FAILURE or SUCCESS on success and set data pointer
 */
int hi_get(const hi_handle_t *hi_handle, const void *key, uint32_t keylen, void **data)
{

	switch (hi_handle->coll_eng) {

		case COLL_ENG_LIST:
		case COLL_ENG_LIST_HASH:
		case COLL_ENG_LIST_MTF:
		case COLL_ENG_LIST_MTF_HASH:
			return lhi_get_list(hi_handle, key, keylen, data);

		case COLL_ENG_RBTREE:
			return lhi_get_rbtree(hi_handle, key, keylen, data);
		/* FIXME */
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			return lhi_get_array(hi_handle, key, keylen, data);
		default:
			return HI_ERR_INTERNAL;
	}

	/* catch rule */
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
int hi_remove(hi_handle_t *hi_handle, void *key, uint32_t keylen, void **data)
{
	switch (hi_handle->coll_eng) {

		case COLL_ENG_LIST:
		case COLL_ENG_LIST_HASH:
		case COLL_ENG_LIST_MTF:
		case COLL_ENG_LIST_MTF_HASH:
			return lhi_remove_list(hi_handle, key, keylen, data);

		case COLL_ENG_RBTREE:
			return lhi_remove_rbtree(hi_handle, key, keylen, data);

		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			return lhi_remove_array(hi_handle, key, keylen, data);

		default:
			return HI_ERR_INTERNAL;
	}

	/* catch rule */
	return HI_ERR_INTERNAL;
}


static void do_rehash(hi_handle_t *h)
{
	float lf;

	if (!h->rehash_auto)
		return;

	lf = h->no_objects / h->table_size;

	if (lf > h->rehash_threshold)
		hi_rehash(h, h->table_size * 2);
}


/**
 * hi_insert insert a key/data pair into our hashhandle
 *
 * @arg hi_handle the hashish handle
 * @return SUCCESS or a negativ return values in the case of an error
 */
int hi_insert(hi_handle_t *hi_handle, const void *key, uint32_t keylen, const void *data)
{
	int ret;

	if (hi_lookup(hi_handle, key, keylen) == SUCCESS) /* already in hash or error */
		return HI_ERR_DUPKEY;

	do_rehash(hi_handle);

	switch (hi_handle->coll_eng) {
		case COLL_ENG_LIST:
		case COLL_ENG_LIST_HASH:
		case COLL_ENG_LIST_MTF:
		case COLL_ENG_LIST_MTF_HASH:
			ret = lhi_insert_list((hi_handle_t *)hi_handle, key, keylen, data);
			break;
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			ret = lhi_insert_array((hi_handle_t *)hi_handle, key, keylen, data);
			break;
		case COLL_ENG_RBTREE:
			ret = lhi_insert_rbtree(hi_handle, key, keylen, data);
			break;
		default:
			ret = HI_ERR_INTERNAL;
			break;
	}
	return ret;
}



/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
