/*
** $Id: hashfunc.c 3 2007-05-21 12:38:44Z hgndgtl $
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

#include "privlibhashish.h"

/**
 * lhi_hash_dumb1 a really dumb function which always
 * returns 1.
 */
uint32_t lhi_hash_dumb1(const uint8_t *key, uint32_t len)
{
	(void) key;
	(void) len;

	return 1;
}

/** lhi_hash_elf is the currently used hashing function for resolving symbol
 * names for the UNIX elf environment. It is a variant of
 * the `hashpjw' function by P.J. Weinberger [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools]
 *
 * "In 32-bit environments, the algorithms are identical" (http://www.ddj.com/184409859).
 * adapted from bfd/elf.c, GNU binutils 2.17.
 */

uint32_t lhi_hash_elf(const uint8_t *key, uint32_t len)
{
	uint32_t hash = 0, x = 0, i = 0;

	for (i = 0; i < len; key++, i++) {
		hash = (hash << 4) + (*key);
		if ((x = hash & 0xF0000000L) != 0) {
			hash ^= x >> 24;
          /* The ELF ABI says `h &= ~g', but this is equivalent in
             this case and on some machines one insn instead of two.  */
			hash ^= x;
		}

	}

	return hash;
}

/**
 * lhi_hash_torek  Chris Torek and Dan Bernstein hash function.
 * Degrades badly when size is divisible by 2
 */
uint32_t lhi_hash_torek(const uint8_t *key, uint32_t len)
{
	uint32_t i, hash = 0;

	for (i = 0; i < len; i++) {
		hash *= 33;
		hash += key[i];
	}

	return hash;
}


/**
 * lhi_hash_phongs  Phong Vo hash function.
 */
uint32_t lhi_hash_phong(const uint8_t *key, uint32_t len)
{
	uint32_t i, hash = 0;

	for (i = 0; i < len; i++)
		hash = hash * 129 + (uint32_t)(key[i]) + 987654321L;

	return hash;
}


/**
 * lhi_hash_kr Brian W. Kernighan and Dennis M. Richie's hash function.
 * Fast, but bad ;-)
 */
uint32_t lhi_hash_kr(const uint8_t *key, uint32_t len)
{
	uint32_t i, hash = 0;

	for (i = 0; i < len; i++)
		hash += key[i];

	return hash;
}

uint32_t lhi_hash_sdbm(const uint8_t *key, uint32_t len)
{
	uint32_t i, hash = 0;


	for (i = 0; i < len; i++)
		hash = key[i] + (hash << 6) + (hash << 16) - hash;

	return hash;
}

/* djb2
 * This algorithm was first reported by Dan Bernstein
 * many years ago in comp.lang.c
 * hash * 33 + char
 */
uint32_t lhi_hash_djb2(const uint8_t *key, uint32_t len)
{
	uint32_t i, hash = 5381;

	for (i = 0; i < len; i++)
		hash = ((hash << 5) + hash) + key[i];

	return hash;
}

/**
 * korzendorfer1, a visual feast
 * This great pleasure of hashing algorithms are
 * subtle by Markus Korzendorfer - a fellow student
 * of mine
 */
uint32_t lhi_hash_korzendorfer1(const uint8_t *key, uint32_t len)
{
	uint32_t i, hash = 0;

	for (i = 0; i < len; i++) {
		hash += key[i];
	}

	return hash;

}

/**
 * The same as above, korzendorfer2 an ueber hash
 * algorithm. Only for the purpose of demonstrating
 */
uint32_t lhi_hash_korzendorfer2(const uint8_t *key, uint32_t len)
{
	uint32_t i, j, size, hash = 0;

	size = 1;

	for (i = 0; i < len; i++) {
		j = key[i];
		j *= size++;
		hash += j;
	}

	return hash;
}


uint32_t lhi_hash_xor(const uint8_t *key, uint32_t len)
{
	uint32_t i, hash = 0;

	for (i = 0; i < len; i++)
		hash = hash ^ key[i];

	return hash;
}

#if 0
/* sscli_20021101.tgz
 * Shared Source Common Language Infrastructure 1.0 Release
 * /tmp/sscli/clr/src/csharp/csharp/sccomp/namemgr.cpp */

unsigned NAMEMGR::HashString(LPCWSTR str, int length)
{

    unsigned hash = giHashStart;


    ASSERT(sizeof(DWORD)/sizeof(WCHAR) == 2);

    // Hash two characters/one dword at a time.
    while (length >= 2) {
        length -= 2;
#if BIGENDIAN
        // There's an assumption that the hash values fit into certain
        // ranges so make sure it matches little endian format
        hash += *str | *(str+1) << 16;
#else
        hash += GET_UNALIGNED_32(str);
#endif
        hash += (hash << 10);
        hash ^= (hash >> 6);
        str += 2;
    }

    // Hash the last character, if any.
    if (length > 0) {
        hash += *str;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return HashUInt(hash);
}

#endif


/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */

