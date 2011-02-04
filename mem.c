/* mem.c
 * 
 * Malloc replacement
 * 
 * Created by Oliver Sharma on 2007-03-26.
 * Modified by Joe Sventek on 2009-12-16
 * Modified by Joe Sventek on 2009-12-19 to add str_dupl
 * Copyright (c) 2007. All rights reserved.
 *
 * Based on chapter 5 in book "C Interfaces and Implementations by David R. Hanson"
 */
#include "mem.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int log_allocation = 0;		/* if true, log size on stdout */

void *mem_alloc_location(size_t nbytes, const char *file, int line) {
    void *p;
    	
    p = malloc(nbytes);
    if (p && log_allocation)
        printf("%p allocated @ %s/line %d, %zd bytes\n", p, file, line, nbytes);
    
    return p;
}

void mem_free_location(void *ptr, const char *file, int line) {
    if (ptr) {
        if (log_allocation)
            printf("%p dealloced @ %s/line %d\n", ptr, file, line);
        free(ptr);
    }
}

char *str_dupl_location(const char *s, const char *file, int line) {
    int n = strlen(s) + 1;
    char *p = (char *)mem_alloc_location(n, file, line);
    if (p)
	strcpy(p, s);
    return p;
}

void mem_heap_end_address(char *leadString) {
    printf("%s %p\n", leadString, sbrk(0));
}
