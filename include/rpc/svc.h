#ifndef _RPC_SVC_H
#include <sunrpc/rpc/svc.h>

libc_hidden_proto (xprt_register)
libc_hidden_proto (xprt_unregister)
libc_hidden_proto (svc_register)
libc_hidden_proto (svc_unregister)
libc_hidden_proto (svcerr_auth)
libc_hidden_proto (svcerr_noprog)
libc_hidden_proto (svcerr_progvers)

/* Now define the internal interfaces.  */
extern int registerrpc (u_long prognum, u_long versnum, u_long procnum,
			char *(*progname) (char *), xdrproc_t inproc,
			xdrproc_t outproc);

extern SVCXPRT *svcfd_create (int fd, u_int sendsize, u_int recvsize);

extern int svcudp_enablecache (SVCXPRT *transp, u_long size);
extern SVCXPRT *svcunixfd_create (int fd, u_int sendsize, u_int recvsize);
extern bool_t svc_sendreply_internal (SVCXPRT *xprt, xdrproc_t __xdr_results,
				      caddr_t __xdr_location) attribute_hidden;
extern void svcerr_decode_internal (SVCXPRT *__xprt) attribute_hidden;
extern void svc_getreq_internal (int __rdfds) attribute_hidden;
extern void svc_getreq_common_internal (const int __fd) attribute_hidden;
extern void svc_getreqset_internal (fd_set *__readfds) attribute_hidden;
extern void svc_getreq_poll_internal (struct pollfd *,
				      const int) attribute_hidden;
extern SVCXPRT *svcudp_create_internal (int __sock) attribute_hidden;
extern SVCXPRT *svcudp_bufcreate_internal (int __sock, u_int __sendsz,
					   u_int __recvsz) attribute_hidden;

#endif
