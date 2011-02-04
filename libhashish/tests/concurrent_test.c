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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

#include <pthread.h>

#include "libhashish.h"
#include "tests.h"


#define MAXTHREAD 100

#define	TABLE_SIZE 23
#define	TEST_ITER_NO 2048
#define	KEYLEN  100
#define	DATALEN 100

#define	KEY 0
#define	DATA 1

#undef xassert
/* if 0 -> raise error */
#define	xassert(x)	\
	do {	\
		if(!x) {	\
			fprintf(stderr, "assert failed: %s:%d (function: %s)\n",	\
					__FILE__, __LINE__, __FUNCTION__);		\
			exit(1);	\
		}	\
	} while (0)

hi_handle_t *hi_hndl;


static void msleep(unsigned long milisec)
{
    struct timespec req;

	memset(&req, 0, sizeof(struct timespec));

    time_t sec = milisec / 1000;

    milisec = milisec - ( sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;

    while (nanosleep(&req, &req) == -1)
         continue;
    return;
}

/* concurrent_test do the following:
 *  o It creates and removes randomly entries
 *    whithin the hash table. At the end the count
 *    must be equal to the expected
 */
static void concurrent_test(int num)
{
	int i, ret;
	void *data;
	char *ptr_bucket[TEST_ITER_NO][2];
	struct drand48_data seed_data;
	unsigned long seed;
	long int rand_res;

	seed = get_proper_seed();

	/* init per thread seed */
	srand48_r(seed, &seed_data);


	/* sleep for some amount of time */
	lrand48_r(&seed_data, &rand_res);
	msleep(rand_res % 40000); /* max 4s */

	fprintf(stderr, "+%d", num);

	for (i =0; i < TEST_ITER_NO; i++) {

		int sucess;
		char *key_ptr, *data_ptr;

		/* insert at least TEST_ITER_NO data
		 * sets
		 */
		do {
			sucess = 1;

			random_string(KEYLEN, &key_ptr, &seed_data);
			random_string(DATALEN, &data_ptr, &seed_data);

			ptr_bucket[i][KEY] = key_ptr;
			ptr_bucket[i][DATA] = data_ptr;

			ret = hi_insert(hi_hndl, (void *) key_ptr,
					strlen(key_ptr), (void *) data_ptr);
			if (ret < 0) {
				fprintf(stderr, "# Key (%s) already in hash\n", key_ptr);
				fprintf(stderr, "Error %s\n", ret == HI_ERR_SYSTEM ?
						strerror(errno) : hi_strerror(ret));
				sucess = 0;
				free(key_ptr);
				free(data_ptr);
			}

		} while (!sucess);
	}

	lrand48_r(&seed_data, &rand_res);
	msleep(rand_res % 4000); /* max 4s */

	/* verify storage and cleanup */
	for (i = 0; i < TEST_ITER_NO; i++) {
		ret = hi_remove(hi_hndl, ptr_bucket[i][KEY],
				strlen(ptr_bucket[i][KEY]), &data);
		if (ret == 0) {
			if (data != ptr_bucket[i][DATA]) {
				fprintf(stderr, "Failed: should %s - is %s\n",
						ptr_bucket[i][DATA], (char *) data);
			}

			free(ptr_bucket[i][KEY]);
			free(ptr_bucket[i][DATA]);
		} else {
			fprintf(stderr, "# already deleted\n");
		}

	}

	fprintf(stderr, "-%d", num);
}

static void *thread_main(void *args)
{
	int num = (int) *((int *) args);

	switch(num % 2) {
		case 1:
			concurrent_test(num);
			break;
		case 0:
			concurrent_test(num);
			break;
		default:
			fputs("programmed error\n", stderr);
			exit(23);
			break;
	}
	free(args);
	return NULL;
}



static int test_hashtable(enum coll_eng collision_engine)
{
	int ret = 0, i;
	pthread_t thread_id[MAXTHREAD];
	struct hi_init_set hi_set;

	fputs("# concurrent test\n", stderr);

	/* create one hash */
	hi_set_zero(&hi_set);
	hi_set_bucket_size(&hi_set, 100);
	hi_set_hash_alg(&hi_set, HI_HASH_ELF);
	hi_set_coll_eng(&hi_set, collision_engine);
	hi_set_key_cmp_func(&hi_set, hi_cmp_str);

	ret = hi_create(&hi_hndl, &hi_set);
	if (ret != 0) {
		fprintf(stderr, "Error %s\n", ret == HI_ERR_SYSTEM ?
				strerror(errno) : hi_strerror(ret));
		return ret;
	}

	for (i = 0; i < MAXTHREAD; i++) {
		int *num = malloc(sizeof(int *));

		if (!num) {
			perror("malloc");
			exit(1);
		}
		*num = i;
		ret = pthread_create(&thread_id[i], NULL, thread_main, num);
		if (ret) {
			fprintf(stderr, "pthread_create failed: %s\n", strerror(ret));
			free(num);
			return ret;
		}
	}

	fputs("# + -> thread startup; - -> thread shutdown\n", stderr);
	for(i = 0; i < MAXTHREAD; i++) {
		pthread_join(thread_id[i], NULL);
	}

	hi_fini(hi_hndl);

	fprintf(stderr, " passed\n");
	return ret;
}


int main(int ac, char **av)
{
	(void) ac; (void) av;
#ifndef THREADSAFE
	fputs("WARNING: library compiled without --enable-thread-locking ?!\n", stderr);
#endif

	fputs("# concurrent test: COLL_ENG_RBTREE\n", stderr);
	test_hashtable(COLL_ENG_RBTREE);

	fputs("# concurrent test: COLL_ENG_LIST\n", stderr);
	return test_hashtable(COLL_ENG_LIST);
}



/* vim: set tw=78 ts=4 sw=4 sts=4 ff=unix noet: */
