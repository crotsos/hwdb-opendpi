/*
 * generic hash table iterator interface.
 * (c) 2008 Florian Westphal <fw@strlen.de>.
 *
 * This file is in the Public Domain.
 */

#include <stdlib.h>
#include <string.h>

#include "libhashish.h"
#include "privlibhashish.h"

struct hi_operator {
	hi_handle_t *handle; /* hash table we are iterating */
	size_t bucket;	/* position in the hash table array we are currently looking at */
	struct lhi_bucket_array a;
	/* a.data: array of pointers to the data stored at ->bucket; these pointers are returned by subsequent iterator_getnext() calls. */
	/* a.keys: array of pointers to the data stored at ->bucket; these pointers are returned by subsequent iterator_getnext() calls. */
	/* a.nmemb: position in the **data / **key array we returned last. counter is decremented, if it is 0, get next bucket */
};


int lhi_bucket_array_alloc(struct lhi_bucket_array *a, size_t nmemb)
{
	size_t alloc = nmemb;
	void *mem_d, *mem_k;
	uint32_t *keylen;
	int ret = HI_ERR_RANGE;

	alloc *= sizeof(void*);
	if (alloc / nmemb != sizeof(void *)) /* integer overflow */
		return HI_ERR_RANGE;

	mem_d = malloc(alloc);
	if (!mem_d)
		return HI_ERR_SYSTEM;
	mem_k = malloc(alloc);
	if (!mem_k) {
		free(mem_d);
		return HI_ERR_SYSTEM;
	}
	alloc = nmemb * sizeof(uint32_t);
	if (alloc / nmemb != sizeof(uint32_t))
		goto out_err;

	ret = HI_ERR_SYSTEM;
	keylen = malloc(alloc);
	if (!keylen)
		goto out_err;

	a->keys = mem_k;
	a->keys_length = keylen;

	a->data = mem_d;
	a->nmemb = nmemb;

	return 0;
 out_err:
	free(mem_d);
	free(mem_k);
	return ret;
}


static void lhi_bucket_array_free(struct lhi_bucket_array *a)
{
	free(a->data);
	free(a->keys);
	free(a->keys_length);
	a->data = NULL;
	a->keys = NULL;
	a->keys_length = NULL;
}


static int rbtree_get_next_tree(hi_iterator_t *i)
{
	hi_handle_t *t = i->handle;

	for (;i->bucket < t->table_size ; i->bucket++) {
		int res = lhi_rbtree_bucket_to_array(t, i->bucket, &i->a);
		if (res == 0)
			return 0;
		if (res != HI_ERR_NODATA)
			return res;
	}
	return HI_ERR_NODATA;
}

static int list_get_next_bucket(hi_iterator_t *i)
{
	hi_handle_t *t = i->handle;

	for (;i->bucket < t->table_size ; i->bucket++) {
		int res = lhi_list_bucket_to_array(t, i->bucket, &i->a);
		if (res == 0)
			return 0;
		if (res != HI_ERR_NODATA)
			return res;
	}
	return HI_ERR_NODATA;
}

static int array_get_next_bucket(hi_iterator_t *i)
{
	hi_handle_t *t = i->handle;

	for (;i->bucket < t->table_size ; i->bucket++) {
		int res = lhi_array_bucket_to_array(t, i->bucket, &i->a);
		if (res == 0)
			return 0;
		if (res != HI_ERR_NODATA)
			return res;
	}
	return HI_ERR_NODATA;
}


int hi_iterator_create(hi_handle_t *t, hi_iterator_t **i)
{
	int res = HI_ERR_SYSTEM;

	hi_iterator_t *it = malloc(sizeof(*it));
	if (!it)
		return HI_ERR_SYSTEM;
	it->handle = t;
	it->bucket = 0;
	memset(&it->a, 0, sizeof(&it->a));
	switch (t->coll_eng) {
	case COLL_ENG_LIST:
	case COLL_ENG_LIST_HASH:
	case COLL_ENG_LIST_MTF:
	case COLL_ENG_LIST_MTF_HASH:
		res = list_get_next_bucket(it);
		break;
	case COLL_ENG_ARRAY:
	case COLL_ENG_ARRAY_HASH:
	case COLL_ENG_ARRAY_DYN:
	case COLL_ENG_ARRAY_DYN_HASH:
		res = array_get_next_bucket(it);
		break;
	case COLL_ENG_RBTREE:
		res = rbtree_get_next_tree(it);
		break;
	default:
		res = HI_ERR_INTERNAL;
	}
	if (res == 0)
		*i = it;
	else
		free(it);
	return res;
}

int hi_iterator_reset(hi_iterator_t *i)
{
	int res = 0;
	enum coll_eng e = i->handle->coll_eng;
	i->bucket = 0;
	lhi_bucket_array_free(&i->a);
	switch (e) {
	case COLL_ENG_RBTREE:
		res = rbtree_get_next_tree(i);
		break;
	case COLL_ENG_ARRAY:
	case COLL_ENG_ARRAY_HASH:
	case COLL_ENG_ARRAY_DYN:
	case COLL_ENG_ARRAY_DYN_HASH:
		res = array_get_next_bucket(i);
		break;
	case COLL_ENG_LIST:
	case COLL_ENG_LIST_HASH:
	case COLL_ENG_LIST_MTF:
	case COLL_ENG_LIST_MTF_HASH:
		res = list_get_next_bucket(i);
		break;
	default:
		return HI_ERR_SYSTEM;
	}
	return res;
}


int hi_iterator_getnext(hi_iterator_t *i, void **data, void **key, uint32_t *keylen)
{
	int ret = 0;

	if (i->a.nmemb)
		goto out;
	switch (i->handle->coll_eng) {
	case COLL_ENG_RBTREE:
		lhi_bucket_array_free(&i->a);
		i->bucket++;
		ret = rbtree_get_next_tree(i);
		break;
	case COLL_ENG_LIST:
	case COLL_ENG_LIST_HASH:
	case COLL_ENG_LIST_MTF:
	case COLL_ENG_LIST_MTF_HASH:
		lhi_bucket_array_free(&i->a);
		i->bucket++;
		ret = list_get_next_bucket(i);
		break;
	case COLL_ENG_ARRAY:
	case COLL_ENG_ARRAY_HASH:
	case COLL_ENG_ARRAY_DYN:
	case COLL_ENG_ARRAY_DYN_HASH:
		lhi_bucket_array_free(&i->a);
		i->bucket++;
		ret = array_get_next_bucket(i);
		break;
	default:
		return HI_ERR_INTERNAL;
	}
	if (ret == 0) {
 out:
		i->a.nmemb--;
		*data = i->a.data[i->a.nmemb];
		*key = i->a.keys[i->a.nmemb];
		*keylen = i->a.keys_length[i->a.nmemb];
	}
	return ret;
}


void hi_iterator_fini(hi_iterator_t *i)
{
	lhi_bucket_array_free(&i->a);
	free(i);
}
