/*
 * iterator tests.
 * - Fill array with random key/value pairs
 * - insert all key/value pairs into hash table
 * - use iterator interface to retrieve all key/value pairs
 *    - verify that no item is returned twice
 *    - verify that no item is omitted
 */

#include "libhashish.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


struct key_value_pair {
	char key[64];
	unsigned int data;
	bool already_seen;
};


static void kvpair_tag_as_seen(struct key_value_pair *k, unsigned int data, unsigned int kvpairs_max)
{
	unsigned int count;

	assert(data < kvpairs_max);

	sscanf(k[data].key, "k%x%%*x", &count);
	assert(count == data);
	assert(!k[data].already_seen);
	k[data].already_seen = true;
}

static void kvpairs_fill(struct key_value_pair *k, unsigned int len)
{
	while (len--) {
		snprintf(k[len].key, sizeof(k[len].key), "k%x%%%x", len, rand() );
		k[len].data = len;
	}
}




static void check_iterator(enum coll_eng engine, struct key_value_pair *k, unsigned int len)
{
	int ret;
	void *data_ptr, *key_ptr;
	uint32_t keylen;
	uint32_t old_table_size;
	unsigned int i;
	hi_handle_t *hi_hndl;
	struct hi_init_set hi_set;
	hi_iterator_t *iterator = NULL;

	hi_set_zero(&hi_set);
	ret = hi_set_bucket_size(&hi_set, 100);
	assert(ret == 0);
	ret = hi_set_hash_alg(&hi_set, HI_HASH_ELF);
	assert(ret == 0);
	ret = hi_set_coll_eng(&hi_set, engine);
	assert(ret == 0);
	ret = hi_set_key_cmp_func(&hi_set, hi_cmp_str);
	assert(ret == 0);

	hi_set_rehash_auto(&hi_set, rand() & 1);
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
	assert(ret == 0);

	for (i = 0 ; i < len ; i++ ) {
		ret = hi_insert_str(hi_hndl, k[i].key, &k[i].data);
		assert(ret == 0);
	}

	ret = hi_iterator_create(hi_hndl, &iterator);
	assert(ret == 0);
	assert(iterator);

	for (i = 0; i < 2; i++) {
		unsigned int j;

		for (j = 0 ; j < len ; j++) {
			unsigned data;
			data_ptr = NULL;
			ret = hi_iterator_getnext(iterator, &data_ptr, &key_ptr, &keylen);
			assert(ret == 0);
			assert(data_ptr);
			assert(key_ptr);
			data = *(unsigned int *) data_ptr;
			kvpair_tag_as_seen(k, data, len);
		}
		ret = hi_iterator_getnext(iterator, &data_ptr, &key_ptr, &keylen);
		assert (ret == HI_ERR_NODATA);
		ret = hi_iterator_reset(iterator);
		assert(ret == 0);

		printf("pass %d done without duplicates, checking for missing elements\n", i+1);
		for (j = 0 ; j < len ; j++) {
			assert(k[j].already_seen);
			k[j].already_seen = false;
		}
	}
	hi_iterator_fini(iterator);


	old_table_size = hi_table_size(hi_hndl);
	ret = hi_rehash(hi_hndl, hi_table_size(hi_hndl) / 4);
	assert(ret == 0);
	assert(hi_table_size(hi_hndl) == old_table_size / 4);

	ret = hi_iterator_create(hi_hndl, &iterator);
	assert(ret == 0);
	assert(iterator);

	for (i = 0; i < 2; i++) {
		unsigned int j;

		for (j = 0 ; j < len ; j++) {
			unsigned data;
			data_ptr = NULL;
			ret = hi_iterator_getnext(iterator, &data_ptr, &key_ptr, &keylen);
			assert(ret == 0);
			assert(data_ptr);
			assert(key_ptr);
			data = *(unsigned int *) data_ptr;
			kvpair_tag_as_seen(k, data, len);
		}
		ret = hi_iterator_getnext(iterator, &data_ptr, &key_ptr, &keylen);
		assert (ret == HI_ERR_NODATA);
		ret = hi_iterator_reset(iterator);
		assert(ret == 0);

		printf("pass %d done without duplicates, checking for missing elements\n", i+1);
		for (j = 0 ; j < len ; j++) {
			assert(k[j].already_seen);
			k[j].already_seen = false;
		}
	}
	hi_iterator_fini(iterator);


	for (i = 0 ; i < len ; i++ ) {
		ret = hi_remove_str(hi_hndl, k[i].key, &data_ptr);
		assert(ret == 0);
		assert(&k[i].data == data_ptr);
	}

	ret = hi_fini(hi_hndl);
	assert(ret == 0);

	fputs("passed\n", stdout);
}

static void seed_prng(void)
{
	volatile unsigned int seed;
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd >= 0) {
		read(fd, (void *)  &seed, sizeof(seed));
		close(fd);
	}
	srand(seed);
}



int
main(void)
{
	struct key_value_pair *kvpairs;
	unsigned int kvpairs_max;

	seed_prng();

	kvpairs_max = rand();
	kvpairs_max %= 8192;
	kvpairs_max += 128;

	printf("Iterator test with %u elements\n", kvpairs_max);

	kvpairs = calloc(kvpairs_max, sizeof(*kvpairs));
	assert(kvpairs);

	kvpairs_fill(kvpairs, kvpairs_max);

	puts(" o check COLL_ENG_RBTREE");
	check_iterator(COLL_ENG_RBTREE, kvpairs, kvpairs_max);

	puts(" o check COLL_ENG_LIST");
	check_iterator(COLL_ENG_LIST, kvpairs, kvpairs_max);

	puts(" o check COLL_ENG_ARRAY");
	check_iterator(COLL_ENG_ARRAY, kvpairs, kvpairs_max);

	puts("\nall tests passed - great!");

	free(kvpairs);
	return 0;
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
