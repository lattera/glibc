#ifndef _RPC_MSG_H
#include <sunrpc/rpc/rpc_msg.h>

/* Now define the internal interfaces.  */

extern bool_t xdr_rejected_reply (XDR *xdrs, struct rejected_reply *rr);
extern bool_t xdr_accepted_reply (XDR *xdrs, struct accepted_reply *ar);


#endif
