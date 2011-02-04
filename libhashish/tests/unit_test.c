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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

#include "libhashish.h"
#include "localhash.h"
#include "tests.h"

#define	TESTSTRING "SURVEILLANCE"

#define	TEST_ITER_NO 2048
#define	KEYLEN 50
#define	DATALEN 100

#define	KEY 0
#define	DATA 1

static uint32_t dumb_hash_func(const uint8_t *key,
		uint32_t len)
{
	(void) key;
	(void) len;

	return 1;
}

static void print_error(int ret)
{
	const char *msg;

	if (ret == HI_ERR_SYSTEM)
		msg = strerror(errno);
	else
		msg = hi_strerror(ret);

	fprintf(stderr, "Error: %s\n", msg);
}


static void check_data(const char *data, const char *expected)
{
	if (strcmp(data, expected)) {
		fprintf(stderr, "strcmp failed: got \"%s\", expected \"%s\"\n",
				data, expected);
		exit(1);
	}
}


static void check_preambel_test(void)
{
  int ret;
  hi_handle_t *hi_handle;
  const char *key = "23";
  const char *data = "data element";
  void *data_ptr;

  /* initialize hashish handle */
  hi_init_str(&hi_handle, 23);

  /* insert an key/data pair */
  ret = hi_insert_str(hi_handle, key, data);

  /* search for a pair with a string key and store result */
  hi_get_str(hi_handle, key, &data_ptr);

  /* free the hashish handle */
  hi_fini(hi_handle);

}

static void check_hi_load_factor(void)
{
	int ret;
	hi_handle_t *hi_handle;
	double load_factor;

	fputs(" o check_hi_load_factor test ...", stdout);

	/* initialize hashish handle */
	hi_init_str(&hi_handle, 3);

	ret = hi_insert_str(hi_handle, "1", NULL);
	ret = hi_insert_str(hi_handle, "2", NULL);

	load_factor = hi_table_load_factor(hi_handle);

	assert(load_factor == ((double)3)/2);

	/* free the hashish handle */
	hi_fini(hi_handle);

	fputs("passed\n", stdout);
}


static void check_remove(enum coll_eng engine, enum hash_alg hash_alg)
{
	int ret;
	hi_handle_t *hi_hndl;
	struct hi_init_set hi_set;
	void *data_ret;

	hi_set_zero(&hi_set);
	ret = hi_set_bucket_size(&hi_set, 100);
	assert(ret == 0);
	ret = hi_set_hash_alg(&hi_set, hash_alg);
	assert(ret == 0);
	ret = hi_set_coll_eng(&hi_set, engine);
	assert(ret == 0);
	ret = hi_set_key_cmp_func(&hi_set, hi_cmp_str);
	assert(ret == 0);

	/* we need aditional arguments for ARRAY based engines */
	switch (engine) {
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			ret = hi_set_coll_eng_array_size(&hi_set, 20);
			assert(ret == 0);
			break;
		default:
			break;
	};


	ret = hi_create(&hi_hndl, &hi_set);
	if (ret != 0)
		print_error(ret);
	assert(ret == 0);
	assert(hi_hndl->no_objects == 0);


	ret = hi_insert(hi_hndl, (void *) "key", strlen("key"), "DATA");
	assert(ret == 0);
	assert(hi_hndl->no_objects == 1);
	ret = hi_insert(hi_hndl, (void *) "key2", strlen("key2"), "DATAX");
	assert(ret == 0);
	assert(hi_hndl->no_objects == 2);
	ret = hi_insert(hi_hndl, (void *) "key3", strlen("key3"), "DATAX");
	assert(ret == 0);

	assert(hi_hndl->no_objects == 3);

	/* key already in data structure -> must return 0 (SUCCESS) */
	ret = hi_remove(hi_hndl, (void *) "key", strlen("key"), &data_ret);
	if (ret != 0)
		print_error(ret);
	assert(ret == 0);
	check_data(data_ret, "DATA");
	assert(hi_hndl->no_objects == 2);
	data_ret = NULL;

	ret = hi_remove(hi_hndl, (void *) "key2", strlen("key2"), &data_ret);
	if (ret != 0)
		print_error(ret);
	assert(ret == 0);
	check_data(data_ret, "DATAX");
	assert(hi_hndl->no_objects == 1);
	data_ret = NULL;

	ret = hi_remove(hi_hndl, (void *) "key3", strlen("key3"), &data_ret);
	if (ret != 0)
		print_error(ret);
	assert(ret == 0);
	check_data(data_ret, "DATAX");
	assert(hi_hndl->no_objects == 0);

	/* must fail */
	ret = hi_remove(hi_hndl, (void *) "key", strlen("key"), &data_ret);
	assert(ret == HI_ERR_NOKEY);

	ret = hi_fini(hi_hndl);
	assert(ret == 0);

	fputs("passed\n", stdout);
}


static void check_get_remove(enum coll_eng engine, enum hash_alg hash_alg)
{
	int ret;
	hi_handle_t *hi_hndl;
	struct hi_init_set hi_set;
	void *data_ret;

	hi_set_zero(&hi_set);
	ret = hi_set_bucket_size(&hi_set, 100);
	assert(ret == 0);
	ret = hi_set_hash_alg(&hi_set, hash_alg);
	assert(ret == 0);
	ret = hi_set_coll_eng(&hi_set, engine);
	assert(ret == 0);
	ret = hi_set_key_cmp_func(&hi_set, hi_cmp_str);
	assert(ret == 0);

	/* we need aditional arguments for ARRAY based engines */
	switch (engine) {
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			ret = hi_set_coll_eng_array_size(&hi_set, 20);
			assert(ret == 0);
			break;
		default:
			break;
	};

	ret = hi_create(&hi_hndl, &hi_set);
	if (ret != 0)
		print_error(ret);
	assert(ret == 0);

	assert(hi_hndl->no_objects == 0);
	ret = hi_insert(hi_hndl, (void *) "key", strlen("key"), "DATA");
	assert(ret == 0);
	assert(hi_hndl->no_objects == 1);
	ret = hi_insert(hi_hndl, (void *) "key2", strlen("key2"), "DATAX");
	assert(ret == 0);
	assert(hi_hndl->no_objects == 2);
	ret = hi_insert(hi_hndl, (void *) "key3", strlen("key3"), "DATAX");
	assert(ret == 0);
	assert(hi_hndl->no_objects == 3);

	/* key already in data structure -> must return 0 (SUCCESS) */
	ret = hi_get(hi_hndl, (void *) "key", strlen("key"), &data_ret);
	if (ret != 0)
		print_error(ret);
	check_data(data_ret, "DATA");
	assert(hi_hndl->no_objects == 3);

	ret = hi_get(hi_hndl, (void *) "key3", strlen("key3"), &data_ret);
	if (ret != 0)
		print_error(ret);
	check_data(data_ret, "DATAX");
	assert(hi_hndl->no_objects == 3);
	data_ret = NULL;

	ret = hi_remove(hi_hndl, (void *) "key", strlen("key"), &data_ret);
	assert(ret == 0);
	assert(hi_hndl->no_objects == 2);
	check_data(data_ret, "DATA");
	data_ret = NULL;
	ret = hi_remove(hi_hndl, (void *) "key2", strlen("key2"), &data_ret);
	assert(ret == 0);
	assert(hi_hndl->no_objects == 1);
	ret = hi_remove(hi_hndl, (void *) "key3", strlen("key3"), &data_ret);
	assert(ret == 0);
	assert(hi_hndl->no_objects == 0);
	check_data(data_ret, "DATAX");

	ret = hi_fini(hi_hndl);
	assert(ret == 0);

	fputs("passed\n", stdout);
}


static void check_insert(enum coll_eng engine, enum hash_alg hash_alg)
{
	int ret;
	hi_handle_t *hi_hndl;
	struct hi_init_set hi_set;
	void *data_ptr = (void *) 0xdeadbeef;

	hi_set_zero(&hi_set);
	ret = hi_set_bucket_size(&hi_set, 100);
	assert(ret == 0);
	ret = hi_set_hash_alg(&hi_set, hash_alg);
	assert(ret == 0);
	ret = hi_set_coll_eng(&hi_set, engine);
	assert(ret == 0);
	ret = hi_set_key_cmp_func(&hi_set, hi_cmp_str);
	assert(ret == 0);

	/* we need aditional arguments for ARRAY based engines */
	switch (engine) {
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			ret = hi_set_coll_eng_array_size(&hi_set, 20);
			assert(ret == 0);
			break;
		default:
			break;
	};


	ret = hi_create(&hi_hndl, &hi_set);
	if (ret != 0)
		print_error(ret);
	assert(ret == 0);


	ret = hi_insert(hi_hndl, (void *) "key", strlen("key"), "XX");
	assert(ret == 0);

	/* same key -> must fail */
	ret = hi_insert(hi_hndl, (void *) "key", strlen("key"), "XX");
	assert(ret == HI_ERR_DUPKEY);


	/* key already in data structure -> must return 0 (SUCCESS) */
	ret = hi_get(hi_hndl, (void *) "key", strlen("key"), &data_ptr);
	assert(ret == 0);
	//assert(data_ptr == NULL);

	ret = hi_remove(hi_hndl, (void *) "key", strlen("key"), &data_ptr);
	assert(ret == 0);

	ret = hi_get(hi_hndl, (void *) "key", strlen("key"), &data_ptr);
	assert(ret == HI_ERR_NOKEY);

	ret = hi_fini(hi_hndl);
	assert(ret == 0);

	fputs("passed\n", stdout);
}

static void check_iterator(enum coll_eng engine, enum hash_alg hash_alg)
{
	int ret, i;
	hi_handle_t *hi_hndl;
	struct hi_init_set hi_set;
	hi_iterator_t *iterator;
	void *data_ptr = (void *) 0xdeadbeef;
	void *key;
	uint32_t keylen;

	hi_set_zero(&hi_set);
	ret = hi_set_bucket_size(&hi_set, 100);
	assert(ret == 0);
	ret = hi_set_hash_alg(&hi_set, hash_alg);
	assert(ret == 0);
	ret = hi_set_coll_eng(&hi_set, engine);
	assert(ret == 0);
	ret = hi_set_key_cmp_func(&hi_set, hi_cmp_str);
	assert(ret == 0);

	/* we need aditional arguments for ARRAY based engines */
	switch (engine) {
		case COLL_ENG_ARRAY:
		case COLL_ENG_ARRAY_HASH:
		case COLL_ENG_ARRAY_DYN:
		case COLL_ENG_ARRAY_DYN_HASH:
			ret = hi_set_coll_eng_array_size(&hi_set, 20);
			assert(ret == 0);
			break;
		default:
			break;
	};

	ret = hi_create(&hi_hndl, &hi_set);
	if (ret != 0)
		print_error(ret);
	assert(ret == 0);

	ret = hi_insert(hi_hndl, (void *) "key", strlen("key"), "data");
	assert(ret == 0);

	ret = hi_insert(hi_hndl, (void *) "key1", strlen("key1"), "data1");
	assert(ret == 0);

	ret = hi_insert(hi_hndl, (void *) "key2", strlen("key2"), "data2");
	assert(ret == 0);

	ret = hi_iterator_create(hi_hndl, &iterator);
	if (ret != 0)
		return;

	for (i = 0; i < 2; i++) {
		bool got_key[] = { 0, 0, 0 };
		unsigned int j;

		for (j = 0 ; j < 3 ; j++) {
			data_ptr = NULL;
			ret = hi_iterator_getnext(iterator, &data_ptr, &key, &keylen);
			assert(ret == 0);
			assert(data_ptr);
			if (strcmp(data_ptr, "data") == 0) {
				assert(!got_key[0]);
				got_key[0] = true;
				continue;
			}
			if (strcmp(data_ptr, "data1") == 0) {
				assert(!got_key[1]);
				got_key[1] = true;
				continue;
			}
			assert (strcmp(data_ptr, "data2") == 0);
			assert(!got_key[2]);
			got_key[2] = true;
		}
		ret = hi_iterator_getnext(iterator, &data_ptr, &key, &keylen);
		assert (ret == HI_ERR_NODATA);
		ret = hi_iterator_reset(iterator);
		assert(ret == 0);
	}
	hi_iterator_fini(iterator);

	ret = hi_remove(hi_hndl, (void *) "key", strlen("key"), &data_ptr);
	assert(ret == 0);
	ret = hi_remove(hi_hndl, (void *) "key1", strlen("key1"), &data_ptr);
	assert(ret == 0);
	ret = hi_remove(hi_hndl, (void *) "key2", strlen("key2"), &data_ptr);
	assert(ret == 0);

	ret = hi_fini(hi_hndl);
	assert(ret == 0);

	fputs("passed\n", stdout);
}



static void check_hi_init_set(enum hash_alg hash_alg)
{
	int ret;
	struct hi_init_set hi_set;

	fputs(" o struct set initialize tests ...", stdout);

	hi_set_zero(&hi_set);

	/* test if a hash table size of 100
	 * return the right values */
	ret = hi_set_bucket_size(&hi_set, 100);
	assert(ret == 0);

	/* trigger a failure - hash table size of
	 * 0 is invalid */
	ret = hi_set_bucket_size(&hi_set, 0);
	assert(ret == HI_ERR_RANGE);

	/* test for standard hashing algorithm - must pass */
	ret = hi_set_hash_alg(&hi_set, hash_alg);
	assert(ret == 0);

	ret = hi_set_hash_func(&hi_set, dumb_hash_func);
	assert(ret == 0);

	/* hash algorithm not supported test - must fail */
	ret = hi_set_hash_alg(&hi_set, (unsigned int) -1);
	assert(ret == HI_ERR_NOFUNC);

	/* test for standard collision engine - must pass */
	ret = hi_set_coll_eng(&hi_set, COLL_ENG_LIST);
	assert(ret == 0);

	/* collision engine not supported test - must fail */
	ret = hi_set_coll_eng(&hi_set, (unsigned int) -1);
	assert(ret == HI_ERR_NOFUNC);

	/* test compare functions - must pass */
	ret = hi_set_key_cmp_func(&hi_set, hi_cmp_str);
	assert(ret == 0);

	/* test compare functions - must fail */
	ret = hi_set_key_cmp_func(&hi_set, NULL);
	assert(ret == HI_ERR_NODATA);

	puts(" passed");
}


static void check_str_wrapper(void)
{
	int ret;
	hi_handle_t *hi_handle;
	const char *key = "23";
	const char *data = "data element";
	void *data_ptr;

	fputs(" o check string wrapper functions tests ...", stdout);

	ret = hi_init_str(&hi_handle, 23);
	assert(ret == 0);

	ret = hi_insert_str(hi_handle, key, data);
	assert(ret == 0);

	ret = hi_get_str(hi_handle, key, &data_ptr);
	assert(ret == 0);

	ret = hi_fini(hi_handle);
	assert(ret == 0);

	puts(" passed");
}

static void check_int16_wrapper(void)
{
	int ret;
	hi_handle_t *hi_handle;
	int16_t key = 23;
	int data = 666, *val;
	void *data_ptr;

	fputs(" o check int16_t wrapper functions tests ...", stdout);

	ret = hi_init_int16_t(&hi_handle, 23);
	assert(ret == 0);

	ret = hi_insert_int16_t(hi_handle, key, &data);
	assert(ret == 0);

	ret = hi_get_int16_t(hi_handle, key, &data_ptr);
	assert(ret == 0);
	val = data_ptr;
	assert(*val == data);

	ret = hi_fini(hi_handle);
	assert(ret == 0);

	puts(" passed");
}

static void check_uint16_wrapper(void)
{
	int ret;
	hi_handle_t *hi_handle;
	uint16_t key = 23;
	int data = 666, *val;
	void *data_ptr;

	fputs(" o check int16_t wrapper functions tests ...", stdout);

	ret = hi_init_uint16_t(&hi_handle, 23);
	assert(ret == 0);

	ret = hi_insert_uint16_t(hi_handle, key, &data);
	assert(ret == 0);

	ret = hi_get_uint16_t(hi_handle, key, &data_ptr);
	assert(ret == 0);
	val = data_ptr;
	assert(*val == data);

	ret = hi_fini(hi_handle);
	assert(ret == 0);

	puts(" passed");
}


static void check_int32_wrapper(void)
{
	int ret;
	hi_handle_t *hi_handle;
	int32_t key = 23;
	int data = 666, *val;
	void *data_ptr;

	fputs(" o check int32_t wrapper functions tests ...", stdout);

	ret = hi_init_int32_t(&hi_handle, 23);
	assert(ret == 0);

	ret = hi_insert_int32_t(hi_handle, key, &data);
	assert(ret == 0);

	ret = hi_get_int32_t(hi_handle, key, &data_ptr);
	assert(ret == 0);
	val = data_ptr;
	assert(*val == data);

	ret = hi_fini(hi_handle);
	assert(ret == 0);

	puts(" passed");
}

static void check_uint32_wrapper(void)
{
	int ret;
	hi_handle_t *hi_handle;
	uint32_t key = 23;
	int data = 666, *val;
	void *data_ptr;

	fputs(" o check uint32_t wrapper functions tests ...", stdout);

	ret = hi_init_uint32_t(&hi_handle, 23);
	assert(ret == 0);

	ret = hi_insert_uint32_t(hi_handle, key, &data);
	assert(ret == 0);

	ret = hi_get_uint32_t(hi_handle, key, &data_ptr);
	assert(ret == 0);
	val = data_ptr;
	assert(*val == data);

	ret = hi_fini(hi_handle);
	assert(ret == 0);

	puts(" passed");
}




static void test_backend(enum coll_eng engine, enum hash_alg hash_alg)
{
	fputs("\tcheck insert ... ", stdout); fflush(stdout);
	check_insert(engine, hash_alg);

	fputs("\tcheck remove ... ", stdout); fflush(stdout);
	check_remove(engine, hash_alg);

	fputs("\tcheck get/remove ... ", stdout); fflush(stdout);
	check_get_remove(engine, hash_alg);

	fputs("\tcheck iterator... ", stdout); fflush(stdout);
	check_iterator(engine, hash_alg);
}


int
main(void)
{
	int hash_alg;

	puts("Start test sequence\n");

	check_preambel_test();

	for (hash_alg = 0; hash_alg < HI_HASH_MAX; hash_alg++) {

		check_hi_init_set(hash_alg);


		puts(" o check COLL_ENG_LIST");
		test_backend(COLL_ENG_LIST, hash_alg);

		puts(" o check COLL_ENG_RBTREE");
		test_backend(COLL_ENG_RBTREE, hash_alg);

		puts(" o check COLL_ENG_ARRAY");
		test_backend(COLL_ENG_ARRAY, hash_alg);

	}

	check_str_wrapper();
	check_int16_wrapper();
	check_uint16_wrapper();
	check_int32_wrapper();
	check_uint32_wrapper();
	check_hi_load_factor();

	puts("\nall tests passed - great!");

	return 0;
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
