#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/time.h>

#include "libhashish.h"
#include "analysis_common.h"


static void die_usage(void)
{
	fputs("Usage: hashstr algorithm string\n", stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	hash_function_t fun;

	if (argc != 3)
		die_usage();

	fun = get_hashfunc_by_name(argv[1]);
	if (!fun)
		die_list();

	return printf("%u\n", fun((const uint8_t*)argv[2], strlen(argv[2])))<= 0;
}

