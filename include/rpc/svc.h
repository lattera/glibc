#ifndef _RPC_SVC_H
#include <sunrpc/rpc/svc.h>

/* Now define the internal interfaces.  */
extern int registerrpc (u_long prognum, u_long versnum, u_long procnum,
			char *(*progname) (char *), xdrproc_t inproc,
			xdrproc_t outproc);

extern SVCXPRT *svcfd_create (int fd, u_int sendsize, u_int recvsize);

extern int svcudp_enablecache (SVCXPRT *transp, u_long size);
extern SVCXPRT *svcunixfd_create (int fd, u_int sendsize, u_int recvsize);

#endif
