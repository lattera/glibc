#ifndef _RPC_MSG_H
#include <sunrpc/rpc/rpc_msg.h>

libc_hidden_proto (_seterr_reply)

/* Now define the internal interfaces.  */

extern bool_t xdr_rejected_reply (XDR *xdrs, struct rejected_reply *rr);
extern bool_t xdr_accepted_reply (XDR *xdrs, struct accepted_reply *ar);
extern bool_t xdr_callmsg_internal (XDR *__xdrs, struct rpc_msg *__cmsg);
extern bool_t xdr_callhdr_internal (XDR *__xdrs, struct rpc_msg *__cmsg);
extern bool_t xdr_replymsg_internal (XDR *__xdrs, struct rpc_msg *__rmsg);

#endif
