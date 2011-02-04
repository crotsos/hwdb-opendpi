/*
 * header file for endpoint ADT used in SRPC
 */

#ifndef _ENDPOINT_H_INCLUDED_
#define _ENDPOINT_H_INCLUDED_

#include <netinet/in.h>

/*
 * all data in an endpoint is in NETWORK order;
 * this is true by default for a sockaddr_in
 */
typedef struct rpc_endpoint {
	struct sockaddr_in addr;
	unsigned long subport;
} RpcEndpoint;

/*
 * complete (fill in) the endpoint with the sockaddr and subport
 */
void endpoint_complete(RpcEndpoint *ep, struct sockaddr_in *addr,
		       unsigned long subport);

/*
 * construct an endpoint from the arguments
 */
RpcEndpoint *endpoint_create(struct sockaddr_in *addr, unsigned long subport);

/*
 * construct an endpoint from the remaining arguments
 */
void endpoint_construct(RpcEndpoint *ep, struct sockaddr_in *addr,
		        unsigned long subport);

/*
 * duplicate endpoint
 */
RpcEndpoint *endpoint_duplicate(RpcEndpoint *ep);

/*
 * determine if two endpoints are equal
 */
int endpoint_equal(RpcEndpoint *ep1, RpcEndpoint *ep2);

/*
 * compute hash value for an endpoint
 *
 * returns value in the range of 0..limit-1
 */
unsigned endpoint_hash(RpcEndpoint *ep, unsigned limit);

void endpoint_dump(RpcEndpoint *ep, char *leadString);
#endif /* _ENDPOINT_H_INCLUDED_ */
