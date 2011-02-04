/* mem.h
 * 
 * Malloc replacement
 * 
 * Created by Oliver Sharma on 2007-03-26.
 * Modified by Joe Sventek on 2009-12-16
 * Modified by Joe Sventek on 2009-12-29 to add str_dupl
 * Copyright (c) 2007. All rights reserved.
 *
 * Based on chapter 5 in book "C Interfaces and Implementations by David R. Hanson"
 */
#ifndef _MEM_H_INCLUDED_
#define _MEM_H_INCLUDED_

#include <stdlib.h>

extern int log_allocation;

void *mem_alloc_location(size_t nbytes, const char *file, int line);
void mem_free_location(void *ptr, const char *file, int line);
char *str_dupl_location(const char *s, const char *file, int line);
void mem_heap_end_address(char *leadString);

/* The following macros automatically insert the file and line. USE THEM! */

#define mem_alloc(nbytes) \
	mem_alloc_location((nbytes), __FILE__, __LINE__)

#define mem_free(ptr) \
	((void)(mem_free_location((ptr), __FILE__, __LINE__), (ptr) = 0))

#define str_dupl(s) \
	str_dupl_location((s), __FILE__, __LINE__)

#endif /* _MEM_H_INCLUDED_ */
