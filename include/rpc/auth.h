#ifndef _RPC_AUTH_H
#include <sunrpc/rpc/auth.h>

#define DECLARE_NSS_PROTOTYPES(service)					\
extern enum nss_status _nss_ ##service ## _netname2user			\
                       (char netname[MAXNETNAMELEN + 1], uid_t *uidp,	\
			gid_t *gidp, int *gidlenp, gid_t *gidlist,	\
			int *errnop);

DECLARE_NSS_PROTOTYPES (nis)
DECLARE_NSS_PROTOTYPES (nisplus)

#undef DECLARE_NSS_PROTOTYPES

#endif
