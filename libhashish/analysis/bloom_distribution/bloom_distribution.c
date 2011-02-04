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
#include <sys/stat.h>
#include <inttypes.h>
#include <math.h>

# include <gd.h>
# include <gdfontl.h>
# include <gdfonts.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "privlibhashish.h"
#include "localhash.h"

#define	BLOOM_MAP_BIT_SIZE 256
#define	BLOOM_SQUARE (sqrt(BLOOM_MAP_BIT_SIZE))

struct ip_port_tuple {
	char ip_addr[15 + 1];
	char port[5 + 1];
	uint8_t addr_array[6]; /* 4 byte addr + 2 byte port */
};

#ifdef HAVE_LIBGD
static int ip_port_tuple_gen(struct ip_port_tuple **ret,
		struct drand48_data *seed_data)
{
	char *addr;
	uint16_t port;
	struct ip_port_tuple *ipt;
	struct in_addr ia;
	long int rand_val;

	ipt = malloc(sizeof(struct ip_port_tuple));
	if (!ipt)
		return -1;

	/* IPv4 address */
	lrand48_r(seed_data, &rand_val);
	ia.s_addr = rand_val;
	addr = inet_ntoa(ia);
	memcpy(ipt->ip_addr, addr, strlen(addr) + 1);

	/* Port */
	lrand48_r(seed_data, &rand_val);
	port = rand_val;
	sprintf(ipt->port, "%d", port);
	ipt->port[5] = 0;

	/* IPv4 address and port as flat mem dump */
	memcpy(ipt->addr_array, &ia.s_addr, sizeof(ia.s_addr));
	memcpy(&ipt->addr_array[4], &port, sizeof(port));

	*ret = ipt;

	return 0;
}
#endif

#ifdef HAVE_LIBGD
static void usage(const int retval, const char * const me)
{
	fprintf(stderr, "USAGE: %s options\n", me);
	fputs("options:\n"
		  "\t-o <outdir>\t\t\tthe directory for the newly created images\n"
		  "\t-k <no-hash-functions>\t\tthe number of utilzed hash functions\n"
		  "\t-m <bitvector>\t\t\the size of the bigvector in bits\n"
		  "\t-n <ip-port-tuples>\t\tthe number of added ip/port tupples to the map\n\n"
		  "Example: ./bloom_distribution -o imageout -k 4 -m 256 -n 100 -v\n", stderr);


	exit(retval);
}
#endif /* HAVE_LIBGD */


int main(int argc, char **argv)
{

#ifdef HAVE_LIBGD
	int opt = 0, verbose = 0;
	gdImagePtr im;
	FILE *pngout;
	int black, white, red, yellow;
	int k, m, n = 0;
	char label[64];
	int ret, i, j, current_bit, iter;
	struct ip_port_tuple *ipt_src, *ipt_dst;
	uint32_t x, y;
	char png_filename[128];
	hi_bloom_handle_t *bh;
	struct drand48_data seed_data;
	unsigned long seed;
	char *outdir = NULL;
#endif

#ifndef HAVE_LIBGD

	(void) argc; (void) argv;

	fprintf(stderr, "Sorry, you had no GD support built-in, exiting ...\n");
	exit(1);
#else

	while ((opt = getopt(argc, argv, "o:m:k:n:vh")) != -1) {
		switch (opt) {
		case 'm':
			m = atoi(optarg);
			break;
		case 'k':
			k = atoi(optarg);
			break;
		case 'n':
			n = atoi(optarg);
			break;
		case 'v':
			verbose++;
			break;
		case 'o':
			outdir = strdup(optarg);
			break;
		case 'h':
			usage(0, argv[0]);
			break;
		case '?':
			fprintf(stderr, "No such option: `%c'\n\n", optopt);
			usage(EXIT_FAILURE, argv[0]);
			break;
		}
	}

	if (!outdir) {
		fputs("You must specify a directory for the images (e.g. outdir)\n", stderr);
		usage(EXIT_FAILURE, argv[0]);
		exit(1);
	}
	ret = mkdir(outdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (ret < 0) {
		perror("mkdir");
		exit(1);
	}

	/* change directory */
	ret = chdir(outdir);
	if (ret < 0) {
		fprintf(stderr, "Can't change to directory %s\n", outdir);
		exit(1);
	}

	if (!m || !k || !n) {
		fputs("You didn't specify either k, m or n!\n", stderr);
		usage(EXIT_FAILURE, argv[0]);

	}

	/* init random seed */
	seed = get_proper_seed();
	srand48_r(seed, &seed_data);

	ret = hi_bloom_init_mk(&bh, m, k);
	if (ret < 0) {
		fprintf(stderr, "Can't initialize bloom filter\n");
		exit(1);
	}

#define RECT_SIZE 20
#define RECT_BODER_SIZE 3
#define	Y_OFFSET 20



	for (iter = 0; iter < n; ++iter) {

		uint8_t cumulative_addr_array[12]; /* 2 * ip_port_tuple.addr_array */

		if (verbose)
			fprintf(stderr, "iteration %05d\n", iter);

		/* create a image with max_list_len X table_size */
		im = gdImageCreate((BLOOM_SQUARE * (RECT_SIZE + (RECT_BODER_SIZE * 2)) + 2),
				(BLOOM_SQUARE * (RECT_SIZE + (RECT_BODER_SIZE * 2))) + 2 + Y_OFFSET);


		black = gdImageColorAllocate(im, 0, 0, 0);
		white = gdImageColorAllocate(im, 255, 165, 79);
		red   = gdImageColorAllocate(im, 205, 102, 29);
		yellow   = gdImageColorAllocate(im, 230, 230, 230);

		gdImageFilledRectangle(im, 0, 0,
				(BLOOM_SQUARE * (RECT_SIZE + (RECT_BODER_SIZE * 2)) + 2),
				(BLOOM_SQUARE * (RECT_SIZE + (RECT_BODER_SIZE * 2))) + 2 + Y_OFFSET,
				black);

		ret = ip_port_tuple_gen(&ipt_src, &seed_data);
		if (ret < 0) {
			fprintf(stderr, "Can't get ip or port!\n");
			exit(1);
		}
		ret = ip_port_tuple_gen(&ipt_dst, &seed_data);
		if (ret < 0) {
			fprintf(stderr, "Can't get ip or port!\n");
			exit(1);
		}

		memcpy(&cumulative_addr_array[0], &ipt_src->addr_array, sizeof(ipt_src->addr_array));
		memcpy(&cumulative_addr_array[6], &ipt_dst->addr_array, sizeof(ipt_dst->addr_array));

		/* and add to filter */
		hi_bloom_filter_add(bh, cumulative_addr_array, sizeof(cumulative_addr_array));

		x = 1;
		y = 1;
		current_bit = 0;

		for (i = 0; i < BLOOM_SQUARE; i++) {
			for (j = 0; j < BLOOM_SQUARE; j++) {

				int fillcolor;

				if (hi_bloom_bit_get(bh, current_bit)) {
					fillcolor = red;
				}
				else {
					fillcolor = yellow;
				}

				gdImageFilledRectangle(im, x, y + Y_OFFSET,
						x + RECT_SIZE + RECT_BODER_SIZE,
						y + RECT_SIZE + RECT_BODER_SIZE + Y_OFFSET,
						white);
				gdImageFilledRectangle(im, x + RECT_BODER_SIZE,
						y + RECT_BODER_SIZE + Y_OFFSET,
						x + RECT_SIZE, y + RECT_SIZE + Y_OFFSET,
						fillcolor);

				x += RECT_SIZE + (RECT_BODER_SIZE * 2);
				++current_bit;
			}
			x = 1;
			y += RECT_SIZE + (RECT_BODER_SIZE * 2);
		}


		sprintf(label, "%s:%s -> %s:%s (probability: %f)",
				ipt_src->ip_addr, ipt_src->port, ipt_dst->ip_addr, ipt_dst->port,
				hi_bloom_current_false_positiv_probability(bh));

		free(ipt_dst);
		free(ipt_src);

		gdImageString(im, gdFontSmall, 2 , 4 , (unsigned char*) label, white);

		/* construct filename */
		snprintf(png_filename, sizeof(png_filename), "%05d.png", iter);

		pngout = fopen(png_filename, "wb");
		gdImagePng(im, pngout);
		fclose(pngout);
		gdImageDestroy(im);


	}


	hi_fini_bloom_filter(bh);

#endif /* HAVE_LIBGD */

	return 0;
}



/* vim: set tw=78 ts=4 sw=4 sts=4 ff=unix noet: */
