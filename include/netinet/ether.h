#ifndef _NETINET_ETHER_H
#include <inet/netinet/ether.h>

struct etherent;

#define DECLARE_NSS_PROTOTYPES(service)					\
extern enum nss_status _nss_ ## service ## _setetherent (void);		\
extern enum nss_status _nss_ ## service ## _endetherent (void);		\
extern enum nss_status _nss_ ## service ## _getetherent_r		\
                       (struct etherent *result, char *buffer,		\
			size_t buflen, int *errnop);			\
extern enum nss_status _nss_ ## service ## _gethostton_r		\
                       (const char *name, struct etherent *eth,		\
			char *buffer, size_t buflen, int *errnop);	\
extern enum nss_status _nss_ ## service ## _getntohost_r		\
                       (const struct ether_addr *addr,			\
			struct etherent *eth,				\
			char *buffer, size_t buflen, int *errnop);

DECLARE_NSS_PROTOTYPES (files)
DECLARE_NSS_PROTOTYPES (nis)
DECLARE_NSS_PROTOTYPES (nisplus)

#undef DECLARE_NSS_PROTOTYPES

#endif
