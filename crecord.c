/*
 * source for connection record routines and data structures
 */

#include "crecord.h"
#include "ctable.h"
#include "mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char *statenames[] = {"", "IDLE", "QACK_SENT", "RESPONSE_SENT",
                           "CONNECT_SENT", "QUERY_SENT",
			   "AWAITING_RESPONSE", "TIMEDOUT",
                           "DISCONNECT_SENT", "FRAGMENT_SENT",
                           "FACK_RECEIVED", "FRAGMENT_RECEIVED",
                           "FACK_SENT", "SEQNO_SENT"};

CRecord *crecord_create(RpcEndpoint *ep, unsigned long seqno) {
	CRecord *cr = (CRecord *)mem_alloc(sizeof(CRecord));
	if (cr) {
		cr->next = NULL;
		cr->link = NULL;
		cr->mutex = ctable_getMutex();
		pthread_cond_init(&cr->stateChanged, NULL);
		cr->ep = ep;
		cr->svc = NULL;
		cr->pl = NULL;
		cr->resp = NULL;
		cr->size = 0;
		cr->nattempts = 0;
		cr->ticks = 0;
		cr->ticksLeft = 0;
		cr->state = 0;
		cr->seqno = seqno;
		cr->lastFrag = 0;
		cr->pingsTilPurge = PINGS_BEFORE_PURGE;
		cr->ticksTilPing = TICKS_BETWEEN_PINGS;
	}
	return (cr);
}

void crecord_dump(CRecord *cr, char *leadString) {
	endpoint_dump(cr->ep, leadString);
	fprintf(stderr, "seqno: %ld, state: %s\n", cr->seqno, statenames[cr->state]);
}

void crecord_setState(CRecord *cr, unsigned long state) {
	cr->state = state;
	cr->ticksTilPing = TICKS_BETWEEN_PINGS;
	cr->pingsTilPurge = PINGS_BEFORE_PURGE;
	pthread_cond_broadcast(&cr->stateChanged);
}

void crecord_setPayload(CRecord *cr, void *pl, unsigned size,
		        unsigned short nattempts, unsigned short ticks) {
	if (cr->pl)
		mem_free(cr->pl);
	cr->pl = pl;
	cr->size = size;
	cr->nattempts = nattempts;
	cr->ticks = ticks;
	cr->ticksLeft = ticks;
}

void crecord_setService(CRecord *cr, SRecord *sr) {
	cr->svc = sr;
}

static int matchedState(unsigned long st, unsigned long *states, int n) {
    int i;

    for (i = 0; i < n; i++)
	if (st == states[i])
	    return i;
    return n;
}

unsigned long crecord_waitForState(CRecord *cr, unsigned long *states, int n) {
    int i;

    while ((i = matchedState(cr->state, states, n)) == n)
	pthread_cond_wait(&cr->stateChanged, cr->mutex);
    return states[i];
}

void crecord_destroy(CRecord *cr) {
    if (cr) {
	if (cr->ep)
	    mem_free(cr->ep);
	if (cr->pl)
	    mem_free(cr->pl);
	if (cr->resp)
	    mem_free(cr->resp);
	mem_free(cr);
    }
}
