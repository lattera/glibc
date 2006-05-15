#include <sunrpc/rpc/key_prot.h>

extern bool_t xdr_cryptkeyres_internal (XDR *xdrs, cryptkeyres *objp)
  attribute_hidden;
extern bool_t xdr_keystatus_internal (XDR *, keystatus*) attribute_hidden;
extern bool_t xdr_keybuf_internal (XDR *, keybuf) attribute_hidden;
extern bool_t xdr_cryptkeyarg_internal (XDR *, cryptkeyarg*) attribute_hidden;
extern bool_t xdr_cryptkeyarg2_internal (XDR *, cryptkeyarg2*)
  attribute_hidden;
extern bool_t xdr_key_netstres_internal (XDR *, key_netstres*)
  attribute_hidden;
extern bool_t xdr_key_netstarg_internal (XDR *, key_netstarg*)
  attribute_hidden;
extern bool_t xdr_netnamestr_internal (XDR *, netnamestr*) attribute_hidden;
extern bool_t xdr_unixcred_internal (XDR *, unixcred*) attribute_hidden;
