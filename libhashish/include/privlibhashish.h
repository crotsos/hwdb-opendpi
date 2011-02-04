/*
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

#ifndef _LIB_PRIVHASHISH_H
#define	_LIB_PRIVHASHISH_H

#include "../config.h"
#include "libhashish.h"

#define SUCCESS 0
#define FAILURE -1

#ifdef DEBUG
# include <stdio.h>
# define DEBUGPRINTF( fmt, ... )  fprintf(stderr, "DEBUG: %s:%u - " fmt,  __FILE__, __LINE__, __VA_ARGS__)
# define NDEBUG
#include <assert.h>
#else
# define DEBUGPRINTF( fmt, ... )  do { } while(0)
#endif

/* Forces a function to be always inlined
** 'must inline' - so that they get inlined even
** if optimizing for size
*/
#undef __always_inline
#if __GNUC_PREREQ (3,2)
# define __always_inline __inline __attribute__ ((__always_inline__))
#else
# define __always_inline __inline
#endif

#if __GNUC__ >= 3
#if !defined likely && !defined unlikely
# define likely(x)   __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#endif


#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
# define LHI_NO_EXPORT __attribute__((visibility("hidden")))
#else
# define LHI_NO_EXPORT /*!*/
#endif

#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/* use this macros for .so initialize code */
#define __init __attribute__ ((constructor))
#define __exit __attribute__ ((destructor))

#define min(x,y) ({ \
     typeof(x) _x = (x); \
     typeof(y) _y = (y); \
     (void) (&_x == &_y);        \
     _x < _y ? _x : _y; })

#define max(x,y) ({ \
     typeof(x) _x = (x); \
     typeof(y) _y = (y); \
     (void) (&_x == &_y);        \
     _x > _y ? _x : _y; })

#define min_t(type,x,y) \
  ({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
  ({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

extern int __hi_error(int, const char *, unsigned int, const char *, \
	const char *, ...);

#define hi_error(E, FMT,ARG...) \
	__hi_error(E, __FILE__, __LINE__, __FUNCTION__, FMT, ##ARG)

#define hi_errno(E) hi_error(E, NULL)

#define	LHI_DEFAULT_MEMORY_ALIGN 16

/* align all standard XMALLOC calls on 8-byte aligned memory addresses
 * This corresponds to the standard posix_memalign function */
#define	XMALLOC(memptr, size) xalloc_align(memptr, 8, size)
int xalloc_align(void **, size_t, size_t);
size_t strlcpy(char *, const char *, size_t);

int LHI_NO_EXPORT lhi_fini_internal(hi_handle_t *);

/* libhashish.c */
int lhi_create_vanilla_hdnl(hi_handle_t **);
void LHI_NO_EXPORT lhi_transform_hndl_2_hndl(hi_handle_t *, hi_handle_t *);

struct lhi_bucket_array {
	void **data;	/* malloc'ed array of data ptrs inside hash table */
	void **keys;	/* malloc'ed array of key ptrs inside hash table */
	uint32_t *keys_length;/* malloc'ed array of key lengths */
	size_t nmemb;	/* length of data/key array (keys and data are always 1:1) */
};

int LHI_NO_EXPORT lhi_bucket_array_alloc(struct lhi_bucket_array *a, size_t nmemb);

/* private array manipulation functions */

int LHI_NO_EXPORT lhi_lookup_array(const hi_handle_t *, const void *, uint32_t);
int LHI_NO_EXPORT lhi_fini_array(hi_handle_t *);
int LHI_NO_EXPORT lhi_insert_array(hi_handle_t *, const void *, uint32_t , const void *);
int LHI_NO_EXPORT lhi_get_array(const hi_handle_t *, const void *, uint32_t , void **);
int LHI_NO_EXPORT lhi_remove_array(hi_handle_t *, const void *, uint32_t , void **);
int LHI_NO_EXPORT lhi_array_bucket_to_array(const hi_handle_t *hi_handle, size_t bucket, struct lhi_bucket_array *);

/* to signal the current allocation status we need two markers */
enum {
	BA_NOT_ALLOCATED,
	BA_ALLOCATED
};


/* private list manipulation functions */
int LHI_NO_EXPORT lhi_lookup_list(hi_handle_t *, const void *, uint32_t);
int LHI_NO_EXPORT lhi_insert_list(hi_handle_t *, const void *, uint32_t , const void *);
int LHI_NO_EXPORT lhi_fini_list(hi_handle_t *);
int LHI_NO_EXPORT lhi_get_list(const hi_handle_t *, const void *, uint32_t , void **);
int LHI_NO_EXPORT lhi_remove_list(hi_handle_t *, const void *, uint32_t , void **);
int LHI_NO_EXPORT lhi_list_bucket_to_array(const hi_handle_t *hi_handle, size_t bucket, struct lhi_bucket_array *);

/* private rbtree manipulation functions */
#ifndef LHI_DISABLE_RBTREE
int LHI_NO_EXPORT lhi_insert_rbtree(hi_handle_t *, const void *, uint32_t , const void *);
int LHI_NO_EXPORT lhi_get_rbtree(const hi_handle_t *, const void *, uint32_t, void **);
int LHI_NO_EXPORT lhi_remove_rbtree(hi_handle_t *, const void *, uint32_t, void **);
int LHI_NO_EXPORT lhi_fini_rbtree(hi_handle_t *);
int LHI_NO_EXPORT lhi_rbtree_bucket_to_array(const hi_handle_t *hi_handle, size_t, struct lhi_bucket_array *);
#else
static inline int lhi_insert_rbtree(hi_handle_t __attribute__((unused)) *h,
		const void __attribute__((unused))*k,
		uint32_t __attribute__((unused)) l,
		const void __attribute__((unused)) *d)
{
	return HI_ERR_INTERNAL;
}
static inline int lhi_get_rbtree(const hi_handle_t __attribute__((unused)) *h,
		const void __attribute__((unused)) *k,
		uint32_t __attribute__((unused)) l,
		void __attribute__((unused)) **d)
{
	return HI_ERR_INTERNAL;
}
static inline int lhi_remove_rbtree(hi_handle_t __attribute__((unused)) *h,
		const void __attribute__((unused)) *k,
		uint32_t __attribute__((unused)) l,
		void __attribute__((unused)) **d)
{
	return HI_ERR_INTERNAL;
}
static inline int lhi_fini_rbtree(hi_handle_t __attribute__((unused)) *h) { return HI_ERR_INTERNAL;}
static inline int lhi_rbtree_bucket_to_array(const hi_handle_t __attribute__((unused)) *hi_handle,
		size_t __attribute__((unused)) l,
		struct lhi_bucket_array __attribute__((unused)) *a)
{
	return HI_ERR_INTERNAL;
}
#endif
#ifdef __cplusplus
}
#endif

#endif /* _LIB_PRIVHASHISH_H */

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
