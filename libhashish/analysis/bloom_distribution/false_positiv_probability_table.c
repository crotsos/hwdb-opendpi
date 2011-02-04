/*
** $Id$
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <math.h>

#include <gd.h>
#include <gdfontl.h>
#include <gdfonts.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "privlibhashish.h"
#include "localhash.h"

int main(void)
{
	uint32_t m, n, k;

	printf("# x:n y:k z:probability\n");

	m = 128;

	for (n = 1; n < 128; n++) {
		for (k = 1; k < 23; k++) {

			printf("%u %u %f\n", n, k, hi_bloom_false_positiv_probability(m, n, k));
		}
		puts("\n");
	}

	return 0;
}



/* vim: set tw=78 ts=4 sw=4 sts=4 ff=unix noet: */

