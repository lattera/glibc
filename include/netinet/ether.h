#ifndef _NETINET_ETHER_H
#include <inet/netinet/ether.h>

libc_hidden_proto (ether_aton_r)
libc_hidden_proto (ether_ntoa_r)

/* Because the `ethers' lookup does not fit so well in the scheme we
   define a dummy struct here which helps us to use the available
   functions.  */
struct etherent
{
  const char *e_name;
  struct ether_addr e_addr;
};

struct parser_data;
extern int _nss_files_parse_etherent (char *line, struct etherent *result,
				      struct parser_data *data,
				      size_t datalen, int *errnop);
libnss_files_hidden_proto (_nss_files_parse_etherent)

#define DECLARE_NSS_PROTOTYPES(service)					      \
extern enum nss_status _nss_ ## service ## _setetherent (int __stayopen);     \
extern enum nss_status _nss_ ## service ## _endetherent (void);		      \
extern enum nss_status _nss_ ## service ## _getetherent_r		      \
                       (struct etherent *result, char *buffer,		      \
			size_t buflen, int *errnop);			      \
extern enum nss_status _nss_ ## service ## _gethostton_r		      \
                       (const char *name, struct etherent *eth,		      \
			char *buffer, size_t buflen, int *errnop);	      \
extern enum nss_status _nss_ ## service ## _getntohost_r		      \
                       (const struct ether_addr *addr,			      \
			struct etherent *eth,				      \
			char *buffer, size_t buflen, int *errnop);

DECLARE_NSS_PROTOTYPES (files)
DECLARE_NSS_PROTOTYPES (nis)
DECLARE_NSS_PROTOTYPES (nisplus)

#undef DECLARE_NSS_PROTOTYPES

#endif
