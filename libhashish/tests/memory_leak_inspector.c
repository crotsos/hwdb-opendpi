/*
** $Id: concurrent_test.c 12 2007-08-22 07:27:29Z hgndgtl $
**
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


/* Memory Leak Profile - HowTo
 *
 * 1) Install Valgrind
 *
 *   a) Debian: aptitude install valgrind
 *
 * 2) Run valgrind
 *
 *   a) Compile testsuite with debug information
 *      ./configure --enable-debug && make
 *   b) valgrind --tool=memcheck --leak-check=yes ./memory_leak_inspector
 *   c) Search for a line like "...blocks are definitely lost in loss record..."
 *      If there isn't something like the line above -> perfect! ;-)
 *
 *
 */

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


#define MAXTHREAD 10

#define	TABLE_SIZE 23
#define	TEST_ITER_NO 2048
#define	KEYLEN  5
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

hi_handle_t *hi_handle;

static void insert_much_and_free_loop(enum coll_eng engine)
{
	int i, ret;
	void *data;
	char *ptr_bucket[TEST_ITER_NO][2];
	struct drand48_data seed_data;
	unsigned long seed;

	hi_handle_t *hi_hndl;
	struct hi_init_set hi_set;

	hi_set_zero(&hi_set);
	hi_set_bucket_size(&hi_set, 100);
	hi_set_hash_alg(&hi_set, HI_HASH_ELF);
	hi_set_coll_eng(&hi_set, engine);
	hi_set_key_cmp_func(&hi_set, hi_cmp_str);


	/* init per thread seed */
	seed = get_proper_seed();
	srand48_r(seed, &seed_data);

	ret = hi_create(&hi_hndl, &hi_set);
	if (ret < 0)
		fprintf(stderr, "Error %s\n", ret == HI_ERR_SYSTEM ?
				strerror(errno) : hi_strerror(ret));

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

			ret = hi_insert(hi_hndl, key_ptr, strlen(key_ptr), data_ptr);
			if (ret != 0) {
				sucess = 0;
				free(key_ptr);
				free(data_ptr);
			}

		} while (!sucess);
	}

	/* verify storage and cleanup */
	for (i = 0; i < TEST_ITER_NO; i++) {


		ret = hi_remove(hi_hndl, ptr_bucket[i][KEY], strlen(ptr_bucket[i][KEY]), &data);
		if (ret == 0) {
			if (data != ptr_bucket[i][DATA]) {
				fprintf(stderr, "Failed: should %s - is %s\n",
						(char *)ptr_bucket[i][DATA], (char *) data);
			}

			free(ptr_bucket[i][KEY]);
			free(ptr_bucket[i][DATA]);
		} else {
			fprintf(stderr, "#  already deleted\n");
		}
	}

	hi_fini(hi_hndl);

	return;
}


static void do_test(enum coll_eng engine)
{
	int i = 10;

	while (i--)
		insert_much_and_free_loop(engine);
}

int main(int ac, char **av)
{
	(void) ac;
	(void) av;

	fputs("# memory leak inspector\n", stderr);

	//init_seed();

	fputs("testing COLL_ENG_LIST\n", stderr);
	do_test(COLL_ENG_LIST);
	fputs("testing COLL_ENG_RBTREE\n", stderr);
	do_test(COLL_ENG_RBTREE);

	fprintf(stderr, "... finished\n");

	return 0;
}



/* vim: set tw=78 ts=4 sw=4 sts=4 ff=unix noet: */
