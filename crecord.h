/*
 * interface and data structures associated with connection records
 */
#ifndef _CRECORD_H_INCLUDED_
#define _CRECORD_H_INCLUDED_

#include "endpoint.h"
#include "stable.h"
#include <pthread.h>

#define ST_IDLE	1
#define ST_QACK_SENT 2
#define ST_RESPONSE_SENT 3
#define ST_CONNECT_SENT 4
#define ST_QUERY_SENT 5
#define ST_AWAITING_RESPONSE 6
#define ST_TIMEDOUT 7
#define ST_DISCONNECT_SENT 8
#define ST_FRAGMENT_SENT 9
#define ST_FACK_RECEIVED 10
#define ST_FRAGMENT_RECEIVED 11
#define ST_FACK_SENT 12
#define ST_SEQNO_SENT 13

#define TICKS_BETWEEN_PINGS (60 * 50)	/* 1 minute */
#define PINGS_BEFORE_PURGE 3

extern const char *statenames[];

typedef struct c_record {
	struct c_record *next;
	struct c_record *link;
	unsigned long seqno;
	unsigned long state;
	pthread_mutex_t *mutex;
	pthread_cond_t stateChanged;
	RpcEndpoint *ep;
	SRecord *svc;
	void *pl;
	void *resp;
	unsigned size;
	unsigned short nattempts;
	unsigned short ticks;
	unsigned short ticksLeft;
	unsigned short pingsTilPurge;
	unsigned short ticksTilPing;
	unsigned char lastFrag;
} CRecord;

/*
 * create a new connection record
 *
 * returns NULL if error
 */
CRecord *crecord_create(RpcEndpoint *ep, unsigned long seqno);

/*
 * set the connection record state; signals the condition variable, as well
 */
void crecord_setState(CRecord *cr, unsigned long state);

/*
 * set the connection record payload
 */
void crecord_setPayload(CRecord *cr, void *payload, unsigned size,
		        unsigned short nattempts, unsigned short ticks);

/*
 * set the connection record service
 */
void crecord_setService(CRecord *cr, SRecord *sr);

/*
 * wait until connection record reaches one of supplied states
 */
unsigned long crecord_waitForState(CRecord *cr, unsigned long *states, int n);

/*
 * destroy a CRecord
 */
void crecord_destroy(CRecord *cr);

void crecord_dump(CRecord *cr, char *leadString);
#endif /* _CRECORD_H_INCLUDED_ */
