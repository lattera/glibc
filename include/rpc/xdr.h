#include <sunrpc/rpc/xdr.h>

extern bool_t xdr_void_internal (void) attribute_hidden;
extern bool_t xdr_bool_internal (XDR *__xdrs, bool_t *__bp) attribute_hidden;
extern bool_t xdr_u_hyper_internal (XDR *__xdrs, u_quad_t *__ullp)
  attribute_hidden;
extern bool_t xdr_u_long_internal (XDR *__xdrs, u_long *__ulp)
  attribute_hidden;
extern bool_t xdr_u_int_internal (XDR *__xdrs, u_int *__up)
  attribute_hidden;
extern bool_t xdr_u_short_internal (XDR *__xdrs, u_short *__usp)
  attribute_hidden;
extern bool_t xdr_hyper_internal (XDR *__xdrs, quad_t *__ullp)
  attribute_hidden;
extern bool_t xdr_long_internal (XDR *__xdrs, long *__ulp)
  attribute_hidden;
extern bool_t xdr_int_internal (XDR *__xdrs, int *__up)
  attribute_hidden;
extern bool_t xdr_short_internal (XDR *__xdrs, short *__usp)
  attribute_hidden;
extern bool_t xdr_enum_internal (XDR *__xdrs, enum_t *__ep)
  attribute_hidden;
extern bool_t xdr_union_internal (XDR *__xdrs, enum_t *__dscmp, char *__unp,
				  const struct xdr_discrim *choices,
				  xdrproc_t dfault) attribute_hidden;
extern bool_t xdr_string_internal (XDR *__xdrs, char **__cpp, u_int __maxsize);
extern bool_t xdr_array_internal (XDR * _xdrs, caddr_t *__addrp,
				  u_int *__sizep, u_int __maxsize,
				  u_int __elsize, xdrproc_t __elproc)
  attribute_hidden;
extern bool_t xdr_reference_internal (XDR *__xdrs, caddr_t *__xpp,
				      u_int __size, xdrproc_t __proc)
  attribute_hidden;
extern bool_t xdr_bytes_internal (XDR *xdrs, char **cpp, u_int *sizep,
				  u_int maxsize) attribute_hidden;
extern bool_t xdr_netobj_internal (XDR *__xdrs, struct netobj *__np)
  attribute_hidden;
extern bool_t xdr_opaque_internal (XDR *__xdrs, caddr_t __cp, u_int __cnt)
  attribute_hidden;
extern void xdrrec_create_internal (XDR *__xdrs, u_int __sendsize,
				    u_int __recvsize, caddr_t __tcp_handle,
				    int (*__readit) (char *, char *, int),
				    int (*__writeit) (char *, char *, int))
  attribute_hidden;
extern void xdrmem_create_internal (XDR *, const caddr_t, u_int, enum xdr_op)
  attribute_hidden;
extern bool_t xdrrec_endofrecord_internal (XDR *__xdrs, bool_t __sendnow)
  attribute_hidden;
extern bool_t xdrrec_skiprecord_internal (XDR *__xdrs)
  attribute_hidden;
extern bool_t xdrrec_eof_internal (XDR *__xdrs)
  attribute_hidden;

libc_hidden_proto (xdrstdio_create)
