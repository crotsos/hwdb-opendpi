/*
 * Taken from lookup2.c by Bob Jenkins.  (http://burtleburtle.net/bob/c/lookup2.c).
 * --------------------------------------------------------------------
 * lookup2.c, by Bob Jenkins, December 1996, Public Domain.
 * hash(), hash2(), hash3, and mix() are externally useful functions.
 * Routines to test the hash are included if SELF_TEST is defined.
 * You can use this free for any purpose.  It has no warranty.
 * --------------------------------------------------------------------
 */


#include "libhashish.h"


/* NOTE: Arguments are modified. */
#define __jhash_mix(a, b, c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}


static uint32_t
jenkins_hash(const uint8_t *k, uint32_t length, uint32_t initval)
{
	/* k: the key
	 * length: length of the key
	 * initval: the previous hash, or an arbitrary value
	 */
	uint32_t a, b, c, len;

	/* Set up the internal state */
	len = length;
	a = b = 0x9e3779b9;	/* the golden ratio; an arbitrary value */
	c = initval;		/* the previous hash value */

	/* handle most of the key */
	while (len >= 12) {
		a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16) +((uint32_t)k[3]<<24));
		b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16) +((uint32_t)k[7]<<24));
		c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)+((uint32_t)k[11]<<24));

		__jhash_mix(a,b,c);

		k += 12; len -= 12;
	}

	c += length;
	switch(len) {
		case 11: c+=((uint32_t)k[10]<<24);
		case 10: c+=((uint32_t)k[9]<<16);
		case 9 : c+=((uint32_t)k[8]<<8);
		/* the first byte of c is reserved for the length */
		case 8 : b+=((uint32_t)k[7]<<24);
		case 7 : b+=((uint32_t)k[6]<<16);
		case 6 : b+=((uint32_t)k[5]<<8);
		case 5 : b+=k[4];
		case 4 : a+=((uint32_t)k[3]<<24);
		case 3 : a+=((uint32_t)k[2]<<16);
		case 2 : a+=((uint32_t)k[1]<<8);
		case 1 : a+=k[0];
	}
	__jhash_mix(a,b,c);

	return c;
}



uint32_t lhi_hash_jenkins2(const uint8_t *data, uint32_t len)
{
	return jenkins_hash(data, len, 0);
}
