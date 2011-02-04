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

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "libhashish.h"
#include "localhash.h"

/**
 * Generate a radom string based on lrand48_r()
 *
 * @str_len is the length of the string
 * @string is the newly generated string
 * @r_d is the seed value for lrand48_r()
 * @returns 0 on success or a negative value if an
 * error occurred
 */
int random_string(uint32_t str_len, char **string,
		struct drand48_data *r_d)
{
	uint32_t i, retval = 0;
	char *newstring;
	long int result;

	newstring = malloc(str_len + 1);
	if (newstring == NULL) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		return -1;
	}


	for (i = 0; i <= str_len; i++) {
		lrand48_r(r_d, &result);
		newstring[i] = (((int)result) % (122 - 97 + 1)) + 97;
	}
	newstring[str_len] = '\0';

	*string = newstring;

	return retval;
}

/**
 * gaussian provides a normal distribution based
 * on random values. The parameters change the standard
 * behaviour from gaussian distribution to normal one.
 *
 * @deviation is the deviation of the distribution
 * @mean denotes to the mean
 * @r_d is the used seed (to guarantee thread safety)
 *
 * @returns the normal distribution
 */
double gaussian(double deviation, double mean,
		struct drand48_data *r_d)
{
	double rand_res, u1, u2, g1, g2, w;

	do {
		drand48_r(r_d, &rand_res);
		u1 = (rand_res * 2) - 1;

		drand48_r(r_d, &rand_res);
		u2 = (rand_res * 2) - 1;

		w = u1 * u1 + u2 *u2;

	} while ( w >= 1);

	w = sqrt((-2.0 * log(w)) / w);
	g2 = u1 * w;
	g1 = u2 * w;

	return g1 * deviation + mean;
}

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
