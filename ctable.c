/*
 * source for connection record routines and data structures
 */

#include "ctable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define CTABLE_SIZE 31

static CRecord *ctable[CTABLE_SIZE];	/* bucket listheads */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned short ctr = 0;

void ctable_lock(void) {
    pthread_mutex_lock(&mutex);
}

void ctable_unlock(void) {
    pthread_mutex_unlock(&mutex);
}

pthread_mutex_t *ctable_getMutex(void) {
    return &mutex;
}

void ctable_init(void) {
    int i;

    pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < CTABLE_SIZE; i++)
        ctable[i] = NULL;
}

unsigned long ctable_newSubport(void) {
    unsigned long subport;
    pid_t pid;

    if (++ctr > 0x7fff)
        ctr = 1;
    pid = getpid();
    subport = (pid & 0xffff) << 16 | ctr;
    return subport;
}

void ctable_insert(CRecord *cr) {
    unsigned hash = endpoint_hash(cr->ep, CTABLE_SIZE);
#ifdef DEBUG
    crecord_dump(cr, "ctable_insert");
#endif /* DEBUG */
    cr->next = ctable[hash];
    ctable[hash] = cr;
#ifdef DEBUG
    ctable_dump("ctable_dump  ");
#endif /* DEBUG */
}

CRecord *ctable_lookup(RpcEndpoint *ep) {
    unsigned hash = endpoint_hash(ep, CTABLE_SIZE);
    CRecord *r, *ans = NULL;

    for (r = ctable[hash]; r != NULL; r = r->next)
        if (endpoint_equal(ep, r->ep)) {
            ans = r;
            break;
        }
#ifdef DEBUG
    if(ans)
        endpoint_dump(ep, "lookup- found:");
    else
        endpoint_dump(ep, "lookup-!found:");
#endif /* DEBUG */
    return ans;
}

void ctable_remove(CRecord *cr) {
    CRecord *pr, *cu;
    unsigned hash = endpoint_hash(cr->ep, CTABLE_SIZE);

    for (pr = NULL, cu = ctable[hash]; cu != NULL; pr = cu, cu = pr->next) {
	if (cr == cu) {
            if (pr == NULL)
                ctable[hash] = cu->next;
            else
                pr->next = cu->next;
            break;
        }
    }
#ifdef DEBUG
    crecord_dump(cr, "ctable_remove");
#endif /* DEBUG */
}

void ctable_scan(CRecord **retry, CRecord **timed, CRecord **ping, CRecord **purge) {
    CRecord *p, *rty, *tmo, *png, *prg;
    unsigned long st;
    int i;

    rty = NULL;
    tmo = NULL;
    png = NULL;
    prg = NULL;
    for (i = 0; i < CTABLE_SIZE; i++) {
        for (p = ctable[i]; p != NULL; p = p->next) {
            st = p->state;
	    if (st == ST_TIMEDOUT) {
		p->link = prg;
		prg = p;
	    } else if (st == ST_CONNECT_SENT || st == ST_QUERY_SENT
                || st == ST_RESPONSE_SENT || st == ST_DISCONNECT_SENT
		|| st == ST_FRAGMENT_SENT || st == ST_SEQNO_SENT) {
                if (--p->ticksLeft <= 0) {
                    if (--p->nattempts <= 0) {
                        p->link = tmo;
                        tmo = p;
                    } else {
                        p->ticks *= 2;
                        p->ticksLeft = p->ticks;
                        p->link = rty;
                        rty = p;
                    }
                }
	    } else {
		if (--p->ticksTilPing <= 0) {
		    if (--p->pingsTilPurge <= 0) {
			p->link = tmo;
			tmo = p;
#ifdef LOG
			crecord_dump(p, "No pings: ");
#endif /* LOG */
		    } else {
			p->ticksTilPing = TICKS_BETWEEN_PINGS;
			p->link = png;
			png = p;
		    }
		}
	    }
        }
    }
    *retry = rty;
    *timed = tmo;
    *ping = png;
    *purge = prg;
}

void ctable_purge(void) {
    CRecord *p, *next;
    int i;

    (void)pthread_mutex_trylock(&mutex);	/* lock if not already locked */
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    for (i = 0; i < CTABLE_SIZE; i++) {
        for (p = ctable[i]; p != NULL; p = next) {
            next = p->next;
	    crecord_destroy(p);
	}
    }
    ctable_init();
}

void ctable_dump(char *str) {
    CRecord *p;
    int i;

    for (i = 0; i < CTABLE_SIZE; i++) {
        for (p = ctable[i]; p != NULL; p = p->next) {
            crecord_dump(p, str);
        }
    }
}
