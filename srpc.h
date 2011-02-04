/*
 * srpc - a simple UDP-based RPC system to support the Homework database
 */

#ifndef _SRPC_DEFINED_
#define _SRPC_DEFINED_

typedef void *RpcConnection;
typedef void *RpcService;

/* initialize RPC system - bind to ‘port’ if non-zero
 * otherwise port number assigned dynamically
 * returns 1 if successful, 0 if failure */
int rpc_init(unsigned short port);

/* the following methods are used by RPC clients */

/* obtain our ip address (as a string) and port number */
void rpc_details(char *ipaddr, unsigned short *port);

/* send connect message to host:port with initial sequence number
 * svcName indicates the offered service of interest
 * returns 1 after target accepts connect request
 * else returns 0 (failure) */
RpcConnection rpc_connect(char *host, unsigned short port,
		          char *svcName, unsigned long seqno);

/* make the next RPC call, waiting until response received
 * upon successful return, ‘resp’ contains ‘rlen’ bytes of data
 * returns 1 if successful, 0 otherwise */
int rpc_call(RpcConnection rpc, void *query, unsigned qlen,
                        void *resp, unsigned rsize, unsigned *rlen);

/* disconnect from target
 * no return */
void rpc_disconnect(RpcConnection rpc);

/* the following methods are used to offer and withdraw a named service */

/* offer service named `svcName' in this process
 * returns NULL if error */
RpcService *rpc_offer(char *svcName);

/* withdraw service */
void rpc_withdraw(RpcService rps);

/* the following methods are used by a worker thread in an RPC server */

/* obtain the next query message from `rps' - blocks until message available
 * `len' is the size of `qb' to receive the data
 * upon return, rpc has opaque sender information
 *              qb has query data
 *
 * returns actual length as function value
 * returns 0 if there is some massive failure in the system */
unsigned rpc_query(RpcService rps, RpcConnection *rpc, void *qb, unsigned len);

/* send the next response message to the ‘rpc’
 * ‘rb’ contains the response to return to the caller
 * returns 1 if successful
 * returns 0 if there is a massive failure in the system */
int rpc_response(RpcService rps, RpcConnection rpc, void *rb, unsigned len);

/* the following methods are used to prevent parent and child processes from
   colliding over the same port numbers */

/* suspends activities of the RPC state machine by locking the connection
   table */
void rpc_suspend();

/* resumes activities of the RPC state machine by unlocking the connection
   table */
void rpc_resume();

/* reinitializes the RPC state machine: purges the connection table, closes
   the original socket on the original UDP port, creates a new socket and
   binds it to the new port, finally resumes the RPC state machine */
int rpc_reinit(unsigned short port);

#endif /* _SRPC_DEFINED_ */
