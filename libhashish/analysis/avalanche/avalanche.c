
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "libhashish.h"
#include "localhash.h"

#include "analysis_common.h"

static void die_usage(void)
{
	fputs("Usage: avalance algorithm\n"
		"Try: avalance --list for a list of supported algorithms\n", stderr);
	exit(1);
}

static int avalance(hash_function_t func, unsigned int trials, unsigned int repetitions)
{
	uint32_t state;

	unsigned int i, r, j;
	unsigned int size = 32;
	uint32_t save, inb, outb;
	double dTrials = trials;
	char bytes[4];
	int t[32][32];
	double result[32][32];
	struct drand48_data seed_data;
	unsigned long seed;


	/* init random seed */
	seed = get_proper_seed();
	srand48_r(seed, &seed_data);

	memset(result, 0, sizeof(result));
	memset(t, 0, sizeof(t));

	while (trials-- > 0) {

		lrand48_r(&seed_data, (long int *) bytes);

		save = state = (uint32_t) (bytes[0]
				+ 256U * bytes[1]
				+ 65536U * bytes[2]
				+ 16777216U * bytes[3]);

		for (r = 0; r < repetitions; r++)
			state = func((const uint8_t*) &state, sizeof(state));

		inb = state;

		for (i = 0; i < size; i++) {

			state = save ^ (1U << i);

			for (r = 0; r < repetitions; r++)
				state = func((const uint8_t*) &state, sizeof(state));

			/* FIXME: implement something smart */
			outb = state ^ inb;
			for (j = 0; j < size; j++) {
				if ((outb & 1) != 0)
					t[i][j]++;
				outb >>= 1;
			}
		}
	}

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			/* we add a scaling factor of 10 to prepare
			 * the output for visualization (5 is now the best ;-) */
			result[i][j] = (t[i][j] / dTrials) * 10;
			/* 3D-Plot: X Y Z */
			printf("%d %d %f\n", i, j, result[i][j]);
		}
		printf("\n");
	}

	return 0;
}


int main(int argc, char *argv[])
{
	hash_function_t func;

	if (argc != 2)
		die_usage();

	func = get_hashfunc_by_name(argv[1]);

	if (!func)
		die_list();

	avalance(func, 1000000, 1);

	return 0;
}



/* vim: set tw=78 ts=4 sw=4 sts=4 ff=unix noet: */
