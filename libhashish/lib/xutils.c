/*
** $Id: xutils.c 13 2007-08-23 06:46:13Z hgndgtl $
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

//#define _GNU_SOURCE
//#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <string.h>


static struct {
	int code;
	const char *msg;
} lhi_error_values[] = {
	{ HI_ERR_SYSTEM,    "System error" },
	{ HI_ERR_NODATA,    "The given data (argument) is insufficient or wrong - check the arguments!"},
	{ HI_ERR_INTERNAL,  "Internal library error, send a bug report" },
	{ HI_ERR_NOKEY,     "The given key isn't in the data structure" },
	{ HI_ERR_DUPKEY,     "The given key is already in the data structure" },
	{ HI_ERR_NOTIMPL,     "The given functionality isn't implemented in this version" },
	{ HI_ERR_RANGE,     "The given argument is out of range for this function" },
	{ HI_ERR_NOFUNC,     "Functionality not supported" },
};

const char *hi_strerror(int code) {
  register unsigned int i;
  for (i = 0; i < sizeof(lhi_error_values) / sizeof(lhi_error_values[0]); ++i)
    if (lhi_error_values[i].code == code)
      return lhi_error_values[i].msg;

  return "Unknown error";

}


int xalloc_align(void **memptr, size_t alignment, size_t size)
{
#ifdef HAVE_POSIX_MEMALIGN
	return posix_memalign(memptr, alignment, size);
#else
	/* FIXME: here is a workaround necessary - but this needs more
	 * work then my current time window grants (and tomorrow is GPN6 ;-)
	 */
	*memptr = malloc(size);
	return (*memptr == NULL) ? -1 : 0;
#endif
}

size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}



/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
