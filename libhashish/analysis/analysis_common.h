#include <inttypes.h>

#include "libhashish.h"

typedef uint32_t (*hash_function_t)(const uint8_t*, uint32_t);


static void die_list(void)
{
	unsigned int i;

	fputs("Known hash algorithms:\n", stderr);
	for (i=0; i <= HI_HASH_MAX; i++)
		fprintf(stderr, "\t%s\n", lhi_hashfunc_map[i].name);

	exit(1);
}


static hash_function_t get_hashfunc_by_name(const char *name)
{
	unsigned int i;

	for (i=0; i <= HI_HASH_MAX; i++)
		if (strcasecmp(name, lhi_hashfunc_map[i].name) == 0)
			return lhi_hashfunc_map[i].hashfunc;
	return NULL;
}


/* vim: set tw=78 ts=4 sw=4 sts=4 ff=unix noet: */
