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
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>

#include "libhashish.h"
#include "localhash.h"

#include "analysis_common.h"

#define	KEYLEN 10

#define	KS_RANDOM  1
#define	KS_STRING  2
#define	KS_COUNTER 3

/* Chi Square test based on random test suite for GAlib.
 * http://lancet.mit.edu/galib-2.4/examples/randtest.C
 */

static void die_usage(void)
{
	int i;

	fputs("Usage: chi_square <algorithm> <keystrategy> <nchi> <rchi>\n"
		"key strategies:\n"
		"\tstring\n"
		"\tcounter\n"
		"\trandom\n"
		"algorithm:\n", stderr);

	for (i = 0; i <= HI_HASH_MAX; i++)
		fprintf(stderr, "\t%s\n", lhi_hashfunc_map[i].name);

	fputs("nchi: number of probes (e.g. 1000, 10000, ... but should 10 * rchi)\n", stderr);
	fputs("rchi: number of probes (e.g. 100)\n", stderr);
	fputs("\nexample: ./chi_square hsieh string 1000 100\n", stderr);

	exit(1);
}

static uint32_t gen_hashsum_by_random_string(hash_function_t func,
		struct drand48_data *seed_data)
{
	uint32_t ret;
	char *key_ptr;

	random_string(KEYLEN, &key_ptr, seed_data);
	ret = func((uint8_t *)key_ptr, strlen(key_ptr));
	free(key_ptr);

	return ret;
}

static uint32_t gen_hashsum_from_random(hash_function_t func,
		struct drand48_data *seed_data)
{
	long int rand_data;

	lrand48_r(seed_data, &rand_data);

	return func((void*)&rand_data, sizeof(rand_data));
}

static uint32_t gen_hashsum_from_counter(hash_function_t func)
{
	static unsigned long long counter;
	uint32_t ret;

	ret = func((void*)&counter, sizeof(counter));
	counter++;

	return ret;
}


static void chi_square(hash_function_t func, int key_strategy,
		unsigned int nchi, unsigned int rchi, uint32_t probes)
{
	unsigned long i, j;
	struct drand48_data seed_data;
	unsigned long seed;
    double frac = (double) nchi/ (double) rchi;
	double probes_chisq_arr[probes];
	double coll_chisq_res;

	/* init random seed */
	seed = get_proper_seed();
	srand48_r(seed, &seed_data);

	double chisq = 0.0;
	double elimit = 2 * sqrt((double)rchi);

	for (i = 0; i < probes; i++) {
		long f[rchi];
		memset(f, 0, sizeof(f));

		for (j = 0; j < nchi; j++) {

			uint32_t hashsum;

			switch (key_strategy) {
				case KS_RANDOM:
					hashsum = gen_hashsum_from_random(func, &seed_data);
					break;
				case KS_COUNTER:
					hashsum = gen_hashsum_from_counter(func);
					break;
				case KS_STRING:
					hashsum = gen_hashsum_by_random_string(func, &seed_data);
					break;
				default:
					fprintf(stderr, "Programmed error\n");
					exit(1);
					break;

			}

			f[hashsum % rchi]++;
		}

		for (j = 0; j < rchi; j++)
			chisq += ((double)f[j] - frac) * ((double)f[j] - frac);

		chisq *= (double)rchi / (double)nchi;
		probes_chisq_arr[i] = chisq;
#if 0
		fprintf(stdout, "run: %lu chi-square: %lf max: %lf\n", i, chisq, elimit + rchi);

		if (fabs(chisq - rchi) > elimit)
			fprintf(stderr, "Failed\n");
#endif
	}

	/* calcualte cumulative chi square value */
	for (i = 0; i < probes; i++) {
		coll_chisq_res += probes_chisq_arr[i];
	}
	coll_chisq_res /= probes;
	fprintf(stdout, "Cumulative chi-square: %lf max: %lf\n",
			coll_chisq_res, elimit + rchi);
}

static int arg_to_keystrategy(char *arg)
{
	if (!strcmp(arg, "string"))
		return KS_STRING;
	else if (!strcmp(arg, "counter"))
		return KS_COUNTER;
	else if (!strcmp(arg, "random"))
		return KS_RANDOM;
	else
		return -1;
}

int main(int argc, char *argv[])
{
	int key_strategy, nchi, rchi;
	hash_function_t func;

	if (argc != 5)
		die_usage();

	func = get_hashfunc_by_name(argv[1]);
	if (!func)
		die_usage();


	key_strategy = arg_to_keystrategy(argv[2]);
	if (key_strategy < 0)
		die_usage();

	nchi = atoi(argv[3]);
	rchi = atoi(argv[4]);



	chi_square(func, key_strategy, nchi, rchi, 100);

	return 0;
}



/* vim: set tw=78 ts=4 sw=4 sts=4 ff=unix noet: */
