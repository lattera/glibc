#ifndef _RPC_AUTH_H
#include <sunrpc/rpc/auth.h>

libc_hidden_proto (getnetname)
libc_hidden_proto (netname2user)
libc_hidden_proto (host2netname)
libc_hidden_proto (user2netname)
libc_hidden_proto (key_gendes)

/* Now define the internal interfaces.  */
struct key_netstarg;
extern int key_setnet (struct key_netstarg *arg);

#define DECLARE_NSS_PROTOTYPES(service)					\
extern enum nss_status _nss_ ##service ## _netname2user			\
                       (char netname[MAXNETNAMELEN + 1], uid_t *uidp,	\
			gid_t *gidp, int *gidlenp, gid_t *gidlist,	\
			int *errnop);

DECLARE_NSS_PROTOTYPES (nis)
DECLARE_NSS_PROTOTYPES (nisplus)

#undef DECLARE_NSS_PROTOTYPES

extern bool_t xdr_des_block_internal (XDR *__xdrs, des_block *__blkp);
extern bool_t xdr_opaque_auth_internal (XDR *, struct opaque_auth *);

extern AUTH *authunix_create_internal (char *__machname, __uid_t __uid,
				       __gid_t __gid, int __len,
				       __gid_t *__aup_gids) attribute_hidden;
extern AUTH *authunix_create_default_internal (void) attribute_hidden;
extern AUTH *authnone_create_internal (void) attribute_hidden;
extern AUTH *authdes_pk_create_internal (const char *, netobj *, u_int,
					 struct sockaddr *,
					 des_block *) attribute_hidden;

libc_hidden_proto (key_encryptsession_pk)
libc_hidden_proto (key_decryptsession_pk)

#endif
