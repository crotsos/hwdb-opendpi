/*
 * interface and data structures associated table holding offered services
 */
#ifndef _STABLE_H_INCLUDED_
#define _STABLE_H_INCLUDED_

#include "tslist.h"

typedef struct s_record {
    struct s_record *s_next;
    char *s_name;
    TSList s_queue;
} SRecord;

/*
 * initialize the data structures for holding offered services
 */
void stable_init();

/*
 * create a new offered service; return NULL if error or if record exists
 */
SRecord *stable_create(char *serviceName);

/*
 * lookup the service record associated with a particular 8-char service name
 *
 * if successful, returns the associated service record
 * if not, returns NULL
 */
SRecord *stable_lookup(char *name);

/*
 * destroy the service associated with a particular service name
 *
 * there is no return value
 */
void stable_destroy(SRecord *sr);

void stable_dump(void);

#endif /* _STABLE_H_INCLUDED_ */
