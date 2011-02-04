/*
 * interface and data structures for table holding connection records
 *
 * ctable_init(), ctable_lock() assume that the table is not locked
 * ctable_getMutex() works independent of lock status
 * all other methods assume that the table has previously been locked via a
 * call to ctable_lock()
 */
#ifndef _CTABLE_H_INCLUDED_
#define _CTABLE_H_INCLUDED_

#include "endpoint.h"
#include "crecord.h"
#include <pthread.h>

/*
 * lock the connection table
 */
void ctable_lock(void);

/*
 * unlock the connection table
 */
void ctable_unlock(void);

/*
 * return the address of the table mutex
 * needed by crecord_create()
 */
pthread_mutex_t *ctable_getMutex(void);

/*
 * initialize the data structures for holding connection records
 */
void ctable_init(void);

/*
 * issue a new subport for this process
 */
unsigned long ctable_newSubport(void);

/*
 * insert a connection record
 */
void ctable_insert(CRecord *cr);

/*
 * lookup the connection record associated with a particular endpoint
 *
 * if successful, returns the associated connection record
 * if not, returns NULL
 */
CRecord *ctable_lookup(RpcEndpoint *ep);

/*
 * remove a connection record
 *
 * there is no return value
 */
void ctable_remove(CRecord *cr);

/*
 * scan table for timer-based processing
 *
 * for each connection
 *   if ST_TIMEDOUT, thread onto purge list
 *   elsif CONNECT_SENT|QUERY_SENT|RESPONSE_SENT|DISCONNECT_SENT|FRAGMENT_SENT
 *     decrement ticksLeft
 *     if reach 0
 *       decrement nattempts
 *       if reach 0
 *         thread CRecord onto timed linked list
 *       else
 *         double ticks
 *         set ticksLeft to ticks
 *         thread CRecord onto retry linked list
 *   else
 *     decrement ticksTilPing
 *     if reach 0, thread onto ping list
 *       decrement pingsTilPurge
 *       if reach 0
 *         thread Crecord onto timed linked list
 *       else
 *         reset ticksTilPing
 *         thread CRecord onto ping linked list
 * return retry, timed, ping and purge linked lists
 */
void ctable_scan(CRecord **retry, CRecord **timed, CRecord **ping, CRecord **purge);

/*
 * purge all entries from the table
 */
void ctable_purge(void);

void ctable_dump(char *str);

#endif /* _CTABLE_H_INCLUDED_ */
