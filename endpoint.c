/*
 * source file for endpoint ADT used in SRPC
 */

#include "endpoint.h"
#include "mem.h"
#include <stdio.h>
#include <string.h>

static int memeq(void *ss, void *tt, size_t n) {
	unsigned char *s = (unsigned char *)ss;
	unsigned char *t = (unsigned char *)tt;
	while (n-- > 0)
		if (*s++ != *t++)
			return 0;
	return 1;
}

void endpoint_complete(RpcEndpoint *ep, struct sockaddr_in *addr,
		       unsigned long subport) {
		memcpy(&(ep->addr), addr, sizeof(struct sockaddr_in));
		ep->subport = htonl(subport);
}

RpcEndpoint *endpoint_create(struct sockaddr_in *addr, unsigned long subport) {
	RpcEndpoint *ep = (RpcEndpoint *)mem_alloc(sizeof(RpcEndpoint));
	if (ep) {
		endpoint_complete(ep, addr, subport);
	}
	return ep;
}

RpcEndpoint *endpoint_duplicate(RpcEndpoint *ep) {
	RpcEndpoint *nep = (RpcEndpoint *)mem_alloc(sizeof(RpcEndpoint));
	if (nep) {
		memcpy(nep, ep, sizeof(RpcEndpoint));
	}
	return nep;
}

int endpoint_equal(RpcEndpoint *ep1, RpcEndpoint *ep2) {
	int answer = 0;		/* assume not equal */
	if (memeq(&(ep1->addr), &(ep1->addr), sizeof(struct sockaddr_in)))
		if (ep1->subport == ep2->subport)
			answer = 1;
	return answer;
}

#define SHIFT 7		/* should be relatively prime to limit */
unsigned endpoint_hash(RpcEndpoint *ep, unsigned limit) {
	unsigned i;
	unsigned char *p;
	unsigned hash = 0;
	p = (unsigned char *)&(ep->addr);
	for (i = 0; i < sizeof(struct sockaddr_in); i++)
		hash = ((SHIFT * hash) + *p++) % limit;
	hash = ((SHIFT * hash) + ep->subport) % limit;
	return hash;
}

void endpoint_dump(RpcEndpoint *ep, char *leadString) {
	unsigned i;
	unsigned char *p;
	fprintf(stderr, "%s", leadString);
	for (i = 0, p = (unsigned char *)ep; i < sizeof(RpcEndpoint); i++)
		fprintf(stderr, " %02x", *p++);
	fprintf(stderr, "\n");
}
