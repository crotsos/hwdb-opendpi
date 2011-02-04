/*
** $Id: cmp_funcs.c 6 2007-08-20 07:52:21Z hgndgtl $
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

#include <string.h>

#include "privlibhashish.h"

int hi_cmp_str(const uint8_t *key1, const uint8_t *key2)
{
	return strcmp((char *)key1, (char *)key2);
}

int hi_cmp_int32_t(const uint8_t *key1, const uint8_t *key2)
{
	int32_t *a, *b;

	a = (int32_t *) key1;
	b = (int32_t *) key2;

	return *a - *b;
}

int hi_cmp_uint32_t(const uint8_t *key1, const uint8_t *key2)
{
	uint32_t *a, *b;

	a = (uint32_t *) key1;
	b = (uint32_t *) key2;

	return *a - *b;
}

int hi_cmp_int16_t(const uint8_t *key1, const uint8_t *key2)
{
	int16_t *a, *b;

	a = (int16_t *) key1;
	b = (int16_t *) key2;

	return *a - *b;
}

int hi_cmp_uint16_t(const uint8_t *key1, const uint8_t *key2)
{
	uint16_t *a, *b;

	a = (uint16_t *) key1;
	b = (uint16_t *) key2;

	return *a - *b;
}

/* vim:set ts=4 sw=4 tw=78 noet: */
