#include <sunrpc/rpc/xdr.h>

extern bool_t xdr_void_internal (void);
extern bool_t xdr_bool_internal (XDR *__xdrs, bool_t *__bp);
extern bool_t xdr_u_hyper_internal (XDR *__xdrs, u_quad_t *__ullp);
extern bool_t xdr_u_long_internal (XDR *__xdrs, u_long *__ulp);
extern bool_t xdr_u_int_internal (XDR *__xdrs, u_int *__up);
extern bool_t xdr_u_short_internal (XDR *__xdrs, u_short *__usp);
extern bool_t xdr_hyper_internal (XDR *__xdrs, quad_t *__ullp);
extern bool_t xdr_long_internal (XDR *__xdrs, long *__ulp);
extern bool_t xdr_int_internal (XDR *__xdrs, int *__up);
extern bool_t xdr_short_internal (XDR *__xdrs, short *__usp);
extern bool_t xdr_enum_internal (XDR *__xdrs, enum_t *__ep);
extern bool_t xdr_union_internal (XDR *__xdrs, enum_t *__dscmp, char *__unp,
				  const struct xdr_discrim *choices,
				  xdrproc_t dfault);
extern bool_t xdr_string_internal (XDR *__xdrs, char **__cpp, u_int __maxsize);
extern bool_t xdr_array_internal (XDR * _xdrs, caddr_t *__addrp,
				  u_int *__sizep, u_int __maxsize,
				  u_int __elsize, xdrproc_t __elproc);
extern bool_t xdr_reference_internal (XDR *__xdrs, caddr_t *__xpp,
				      u_int __size, xdrproc_t __proc);
extern bool_t xdr_bytes_internal (XDR *xdrs, char **cpp, u_int *sizep,
				  u_int maxsize);
extern bool_t xdr_netobj_internal (XDR *__xdrs, struct netobj *__np);
extern bool_t xdr_opaque_internal (XDR *__xdrs, caddr_t __cp, u_int __cnt);
extern void xdrrec_create_internal (XDR *__xdrs, u_int __sendsize,
				    u_int __recvsize, caddr_t __tcp_handle,
				    int (*__readit) (char *, char *, int),
				    int (*__writeit) (char *, char *, int));
extern void xdrmem_create_internal (XDR *, const caddr_t, u_int, enum xdr_op);
extern bool_t xdrrec_endofrecord_internal (XDR *__xdrs, bool_t __sendnow);
extern bool_t xdrrec_skiprecord_internal (XDR *__xdrs);
extern bool_t xdrrec_eof_internal (XDR *__xdrs);

libc_hidden_proto (xdrstdio_create)
