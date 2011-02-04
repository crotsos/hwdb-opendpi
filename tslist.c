/*
 * tslist.c - implementation of thread-safe linked lists for simple RPC system
 */

#include "tslist.h"
#include "mem.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct element {
	struct element *next;
	void *addr;
	void *data;
	int size;
} Element;
typedef struct listhead {
	Element *head;
	Element *tail;
	unsigned long count;
	pthread_mutex_t mutex;
	pthread_cond_t nonempty;
} ListHead;

/* constructor
 * returns NULL if error */
TSList tsl_create() {
	ListHead *lh = (ListHead *)mem_alloc(sizeof(ListHead));
	if (lh) {
		lh->head = NULL;
		lh->tail = NULL;
		lh->count = 0l;
		if (pthread_mutex_init(&(lh->mutex), NULL) ||
		    pthread_cond_init(&(lh->nonempty), NULL)) {
			mem_free(lh);
			lh = NULL;
		}
	}
	return (TSList)lh;
}

/* append element to the list
 * returns 0 if failure to append (malloc failure), otherwise 1 */
int tsl_append(TSList tsl, void *a, void *b, int size) {
	ListHead *lh = (ListHead *)tsl;
	Element *e = (Element *)mem_alloc(sizeof(Element));
	if (!e)
		return 0;
	e->addr = a;
	e->data = b;
	e->size = size;
	e->next = NULL;
	pthread_mutex_lock(&(lh->mutex));
	if (! lh->count++)
		lh->head = e;
	else
		lh->tail->next = e;
	lh->tail = e;
	pthread_cond_signal(&(lh->nonempty));
	pthread_mutex_unlock(&(lh->mutex));
	return 1;
}

/* prepend element to the list
 * returns 0 if failure to prepend (malloc failure), otherwise 1 */
int tsl_prepend(TSList tsl, void *a, void *b, int size) {
	ListHead *lh = (ListHead *)tsl;
	Element *e = (Element *)mem_alloc(sizeof(Element));
	if (!e)
		return 0;
	e->addr = a;
	e->data = b;
	e->size = size;
	pthread_mutex_lock(&(lh->mutex));
	e->next = lh->head;
	lh->head = e;
	if (! lh->count++)
		lh->tail = e;
	pthread_cond_signal(&(lh->nonempty));
	pthread_mutex_unlock(&(lh->mutex));
	return 1;
}

/* remove first element of the list; thread blocks until successful */
void tsl_remove(TSList tsl, void **a, void **b, int *size) {
	ListHead *lh = (ListHead *)tsl;
	Element *e;

	pthread_mutex_lock(&(lh->mutex));
	while (!lh->count)
		pthread_cond_wait(&(lh->nonempty), &(lh->mutex));
	/*
	 * at this point there is at least one element in the queue
	 */
	e = lh->head;
	lh->head = e->next;
	if (! --lh->count)
		lh->tail = NULL;
	pthread_mutex_unlock(&(lh->mutex));
	*a = e->addr;
	*b = e->data;
	*size = e->size;
	mem_free(e);
}
