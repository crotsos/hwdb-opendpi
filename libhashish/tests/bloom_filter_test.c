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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

#include "privlibhashish.h"
#include "localhash.h"


int
main(void)
{
	int ret;
	hi_bloom_handle_t *bh;

	puts("Start bloom filter test sequence");

	ret = hi_bloom_init_mk(&bh, 256, 4);
	if (ret < 0) {
		if (ret == HI_ERR_SYSTEM)
			perror("failed: hi_bloom_init_mk");
		else
			fprintf(stderr, "failed: hi_bloom_init_mk: %s\n", hi_strerror(ret));

		exit(1);
	}

	hi_bloom_filter_add(bh, (void *) "xxx", 3);
	if ((hi_bloom_filter_check(bh, (void *) "xxx", 3)) != 1) {
		fprintf(stderr, "failed\n");
		exit(1);
	}

	if ((hi_bloom_filter_check_str(bh, "xxx")) != 1) {
		fprintf(stderr, "failed\n");
		exit(1);
	}

	hi_fini_bloom_filter(bh);

	puts("all tests passed");

	return 0;
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
