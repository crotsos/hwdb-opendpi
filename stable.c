/*
 * source for service table and data structures
 */

#include "stable.h"
#include "mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define STABLE_SIZE 13

static SRecord *stable[STABLE_SIZE];	/* bucket listheads */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define SHIFT 7
static int hash(char *s) {
    int ans = 0;

    while (*s != '\0')
	ans = (SHIFT * ans) + *s++;
    return ans % STABLE_SIZE;
}

void stable_init() {
    int i;

    for (i = 0; i < STABLE_SIZE; i++)
        stable[i] = NULL;
}

SRecord *stable_create(char *serviceName) {
    SRecord *r;
    int hv;

    if (stable_lookup(serviceName))
	r = NULL;
    else {
    	r = (SRecord *)mem_alloc(sizeof(SRecord));
    	if (r) {
	    r->s_name = str_dupl(serviceName);
	    r->s_next = NULL;
       	    r->s_queue = tsl_create();
	    if (! r->s_queue) {
		mem_free(r->s_name);
	    	mem_free(r);
	    	r = NULL;
            } else {
		hv = hash(r->s_name);
        	pthread_mutex_lock(&mutex);
        	r->s_next = stable[hv];
        	stable[hv] = r;
        	pthread_mutex_unlock(&mutex);
	    }
	}
    }
    return r;
}

SRecord *stable_lookup(char *name) {
    int hv = hash(name);
    SRecord *r, *ans = NULL;

    pthread_mutex_lock(&mutex);
    for (r = stable[hv]; r != NULL; r = r->s_next)
        if (strcmp(r->s_name, name) == 0) {
            ans = r;
            break;
        }
    pthread_mutex_unlock(&mutex);
    return ans;
}

void stable_remove(SRecord *sr) {
    SRecord *pr, *cu;
    int hv = hash(sr->s_name);

    pthread_mutex_lock(&mutex);
    for (pr = NULL, cu = stable[hv]; cu != NULL; pr = cu, cu = pr->s_next) {
        if (strcmp(sr->s_name, cu->s_name) == 0) {
            if (pr == NULL)
                stable[hv] = cu->s_next;
            else
                pr->s_next = cu->s_next;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    /* should destroy the TSList before deallocating structure */
    mem_free(sr);
}

void stable_dump(void) {
    SRecord *p;
    int i;

    pthread_mutex_lock(&mutex);
    printf("Service table:");
    for (i = 0; i < STABLE_SIZE; i++) {
        for (p = stable[i]; p != NULL; p = p->s_next) {
	    printf(" %s", p->s_name);
        }
    }
    printf("\n");
    pthread_mutex_unlock(&mutex);
}
