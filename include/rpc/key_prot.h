#include <sunrpc/rpc/key_prot.h>

extern bool_t xdr_cryptkeyres_internal (XDR *xdrs, cryptkeyres *objp);
extern bool_t xdr_keystatus_internal (XDR *, keystatus*);
extern bool_t xdr_keybuf_internal (XDR *, keybuf);
extern bool_t xdr_cryptkeyarg_internal (XDR *, cryptkeyarg*);
extern bool_t xdr_cryptkeyarg2_internal (XDR *, cryptkeyarg2*);
extern bool_t xdr_key_netstres_internal (XDR *, key_netstres*);
extern bool_t xdr_key_netstarg_internal (XDR *, key_netstarg*);
extern bool_t xdr_netnamestr_internal (XDR *, netnamestr*);
extern bool_t xdr_unixcred_internal (XDR *, unixcred*);
