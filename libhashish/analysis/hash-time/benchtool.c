#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/time.h>

#include "libhashish.h"
#include "analysis_common.h"

#define TIME_LT(x,y) (x->tv_sec < y->tv_sec || (x->tv_sec == y->tv_sec && x->tv_usec < y->tv_usec))


static void die_usage(void)
{
	fputs("Usage: bench hashfunc [totaldata] [chunksize]\n", stderr);
	exit(1);
}

static void
do_difftime(struct timeval *op1, struct timeval *op2)
{
        int borrow = 0, sign = 0;
        struct timeval *temp_time, res;

        if (TIME_LT(op1, op2)) {
                temp_time = op1;
                op1  = op2;
                op2  = temp_time;
                sign = 1;
        }
        if (op1->tv_usec >= op2->tv_usec) {
                res.tv_usec = op1->tv_usec-op2->tv_usec;
        }
        else {
                res.tv_usec = (op1->tv_usec + 1000000) - op2->tv_usec;
                borrow = 1;
        }
        res.tv_sec = (op1->tv_sec-op2->tv_sec) - borrow;

	printf("%F\n", res.tv_sec + ((double) res.tv_usec) / 1000000);
}


static void get_speed(hash_function_t hashfun, unsigned long len, unsigned long calls)
{
	unsigned i;
	struct timeval tv_then, tv_now;
	uint8_t *mem = malloc(len);
	unsigned long callsleft = calls;

	if (!mem || !calls)
		return;
	sleep(1);

	gettimeofday(&tv_then, NULL); \
	for (i = 0; i < len; i++)
		*mem = (uint8_t) i;

	while (callsleft--)
		hashfun(mem, len);
	gettimeofday(&tv_now, NULL);
	free(mem);
	printf(" %8lu %8lu\t", len, calls);
	do_difftime(&tv_then, &tv_now);
}


int main(int argc, char *argv[])
{
	hash_function_t fun;
	unsigned long databytes = 30000000;
	unsigned long len, calls;

	if (argc < 2)
		die_usage();

	fun = get_hashfunc_by_name(argv[1]);
	if (!fun)
		die_list();

	if (argc == 2) /* only output summary of several get_speed() runs are done */
		printf("# %s\n# length    calls\ttime\n", argv[1]);
	if (argc > 2)
		databytes = atoi(argv[2]);
	if (argc > 3) {
		len = atoi(argv[3]);
		if (len == 0)
			die_usage();
		calls = databytes /= len;
		get_speed(fun, len, calls);
		return 0;
	}
	calls = databytes /= 4;
	get_speed(fun,    4, calls); calls /= 10;
	get_speed(fun,   40, calls); calls /= 10;
	get_speed(fun,  400, calls); calls /= 10;
	get_speed(fun, 4000, calls); calls /= 10;
	get_speed(fun,40000, calls);

	return 0;
}

