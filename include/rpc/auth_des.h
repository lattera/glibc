#ifndef _RPC_AUTH_DES_H

#include <sunrpc/rpc/auth_des.h>

#define DECLARE_NSS_PROTOTYPES(service)					\
extern enum nss_status _nss_ ## service ## _getpublickey		\
                       (const char *netname, char *pkey, int *errnop);	\
extern enum nss_status _nss_ ## service ## _getsecretkey		\
                       (const char *netname, char *skey, char *passwd,	\
			int *errnop);

DECLARE_NSS_PROTOTYPES (files)
DECLARE_NSS_PROTOTYPES (nis)
DECLARE_NSS_PROTOTYPES (nisplus)

#undef DECLARE_NSS_PROTOTYPES

#endif
