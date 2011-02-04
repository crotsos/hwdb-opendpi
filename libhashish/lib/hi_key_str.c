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

#include <string.h>
#include "privlibhashish.h"

/**
 * This is the default initialize function for strings. It takes
 * HI_HASH_DEFAULT as the default hash function, set the compare function for
 * strings and select as the collision engine the list (COLL_ENG_LIST) based
 * one.
 *
 * @arg hi_hndl	this become out new hashish handle
 * @arg table_size dedicates the table size
 * @returns negativ error value or zero on success
 */
int hi_init_str(hi_handle_t **hi_hndl, const uint32_t table_size)
{
	struct hi_init_set hi_set;

	hi_set_zero(&hi_set);
	hi_set_bucket_size(&hi_set, table_size);
	hi_set_hash_alg(&hi_set, HI_HASH_DEFAULT);
	hi_set_coll_eng(&hi_set, COLL_ENG_LIST);
	hi_set_key_cmp_func(&hi_set, hi_cmp_str);

	return hi_create(hi_hndl, &hi_set);
}

int hi_insert_str(hi_handle_t *hi_hndl, const char *key, const void *data)
{
	return hi_insert(hi_hndl, (uint8_t *) key, strlen(key), (void *)data);
}

int hi_get_str(hi_handle_t *hi_hndl, const char *key, void **data)
{
	return hi_get(hi_hndl, (void *)key, strlen(key), data);

}

int hi_remove_str(hi_handle_t *hi_hndl, const char *key, void **data)
{
	return hi_remove(hi_hndl, (void *)key, strlen(key), data);
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
