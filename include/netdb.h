#ifndef	_NETDB_H
#include <resolv/netdb.h>

/* Document internal interfaces.  */
extern int __gethostent_r (struct hostent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct hostent **__restrict __result,
			   int *__restrict __h_errnop);
extern int __old_gethostent_r (struct hostent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct hostent **__restrict __result,
			       int *__restrict __h_errnop);

extern int __gethostbyaddr_r (__const void *__restrict __addr,
			      socklen_t __len, int __type,
			      struct hostent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct hostent **__restrict __result,
			      int *__restrict __h_errnop);
extern int __old_gethostbyaddr_r (__const void *__restrict __addr,
				  socklen_t __len, int __type,
				  struct hostent *__restrict __result_buf,
				  char *__restrict __buf, size_t __buflen,
				  struct hostent **__restrict __result,
				  int *__restrict __h_errnop);

extern int __gethostbyname_r (__const char *__restrict __name,
			      struct hostent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct hostent **__restrict __result,
			      int *__restrict __h_errnop);
extern int __old_gethostbyname_r (__const char *__restrict __name,
				  struct hostent *__restrict __result_buf,
				  char *__restrict __buf, size_t __buflen,
				  struct hostent **__restrict __result,
				  int *__restrict __h_errnop);

extern int __gethostbyname2_r (__const char *__restrict __name, int __af,
			       struct hostent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct hostent **__restrict __result,
			       int *__restrict __h_errnop);
extern int __old_gethostbyname2_r (__const char *__restrict __name, int __af,
				   struct hostent *__restrict __result_buf,
				   char *__restrict __buf, size_t __buflen,
				   struct hostent **__restrict __result,
				   int *__restrict __h_errnop);

extern int __getnetent_r (struct netent *__restrict __result_buf,
			  char *__restrict __buf, size_t __buflen,
			  struct netent **__restrict __result,
			  int *__restrict __h_errnop);
extern int __old_getnetent_r (struct netent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct netent **__restrict __result,
			      int *__restrict __h_errnop);

extern int __getnetbyaddr_r (uint32_t __net, int __type,
			     struct netent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct netent **__restrict __result,
			     int *__restrict __h_errnop);
extern int __old_getnetbyaddr_r (uint32_t __net, int __type,
				 struct netent *__restrict __result_buf,
				 char *__restrict __buf, size_t __buflen,
				 struct netent **__restrict __result,
				 int *__restrict __h_errnop);

extern int __getnetbyname_r (__const char *__restrict __name,
			     struct netent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct netent **__restrict __result,
			     int *__restrict __h_errnop);
extern int __old_getnetbyname_r (__const char *__restrict __name,
				 struct netent *__restrict __result_buf,
				 char *__restrict __buf, size_t __buflen,
				 struct netent **__restrict __result,
				 int *__restrict __h_errnop);

extern int __getservent_r (struct servent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct servent **__restrict __result);
extern int __old_getservent_r (struct servent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct servent **__restrict __result);

extern int __getservbyname_r (__const char *__restrict __name,
			      __const char *__restrict __proto,
			      struct servent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct servent **__restrict __result);
extern int __old_getservbyname_r (__const char *__restrict __name,
				  __const char *__restrict __proto,
				  struct servent *__restrict __result_buf,
				  char *__restrict __buf, size_t __buflen,
				  struct servent **__restrict __result);

extern int __getservbyport_r (int __port,
			      __const char *__restrict __proto,
			      struct servent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct servent **__restrict __result);
extern int __old_getservbyport_r (int __port,
				  __const char *__restrict __proto,
				  struct servent *__restrict __result_buf,
				  char *__restrict __buf, size_t __buflen,
				  struct servent **__restrict __result);

extern int __getprotoent_r (struct protoent *__restrict __result_buf,
			    char *__restrict __buf, size_t __buflen,
			    struct protoent **__restrict __result);
extern int __old_getprotoent_r (struct protoent *__restrict __result_buf,
				char *__restrict __buf, size_t __buflen,
				struct protoent **__restrict __result);

extern int __getprotobyname_r (__const char *__restrict __name,
			       struct protoent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct protoent **__restrict __result);
extern int __old_getprotobyname_r (__const char *__restrict __name,
				   struct protoent *__restrict __result_buf,
				   char *__restrict __buf, size_t __buflen,
				   struct protoent **__restrict __result);

extern int __getprotobynumber_r (int __proto,
				 struct protoent *__restrict __res_buf,
				 char *__restrict __buf, size_t __buflen,
				 struct protoent **__restrict __result);
extern int __old_getprotobynumber_r (int __proto,
				     struct protoent *__restrict __res_buf,
				     char *__restrict __buf, size_t __buflen,
				     struct protoent **__restrict __result);

extern int __getnetgrent_r (char **__restrict __hostp,
			    char **__restrict __userp,
			    char **__restrict __domainp,
			    char *__restrict __buffer, size_t __buflen);

extern int ruserpass (const char *host, const char **aname,
		      const char **apass);


/* The following declarations and definitions have been removed from
   the public header since we don't want people to use them.  */

#define AI_V4MAPPED	0x0008	/* IPv4-mapped addresses are acceptable.  */
#define AI_ALL		0x0010	/* Return both IPv4 and IPv6 addresses.	 */
#define AI_ADDRCONFIG	0x0020	/* Use configuration of this host to choose
				  returned address type.  */
#define AI_DEFAULT    (AI_V4MAPPED | AI_ADDRCONFIG)

#include <inet/netgroup.h>

#define DECLARE_NSS_PROTOTYPES(service)					      \
extern enum nss_status _nss_ ## service ## _setprotoent (int);		      \
extern enum nss_status _nss_ ## service ## _endprotoent (void);		      \
extern enum nss_status _nss_ ## service ## _getprotoent_r		      \
		       (struct protoent *proto, char *buffer, size_t buflen,  \
			int *errnop);					      \
extern enum nss_status _nss_ ## service ## _getprotobyname_r		      \
		       (const char *name, struct protoent *proto,	      \
			char *buffer, size_t buflen, int *errnop);	      \
extern enum nss_status _nss_ ## service ## _getprotobynumber_r		      \
		       (int number, struct protoent *proto,		      \
			char *buffer, size_t buflen, int *errnop);	      \
extern enum nss_status _nss_ ## service ## _sethostent (int);		      \
extern enum nss_status _nss_ ## service ## _endhostent (void);		      \
extern enum nss_status _nss_ ## service ## _gethostent_r		      \
		       (struct hostent *host, char *buffer, size_t buflen,    \
			int *errnop, int *h_errnop);			      \
extern enum nss_status _nss_ ## service ## _gethostbyname2_r		      \
		       (const char *name, int af, struct hostent *host,	      \
			char *buffer, size_t buflen, int *errnop,	      \
			int *h_errnop);					      \
extern enum nss_status _nss_ ## service ## _gethostbyname_r		      \
		       (const char *name, struct hostent *host, char *buffer, \
			size_t buflen, int *errnop, int *h_errnop);	      \
extern enum nss_status _nss_ ## service ## _gethostbyaddr_r		      \
		       (const void *addr, socklen_t addrlen, int af,	      \
			struct hostent *host, char *buffer, size_t buflen,    \
			int *errnop, int *h_errnop);			      \
extern enum nss_status _nss_ ## service ## _setservent (int);		      \
extern enum nss_status _nss_ ## service ## _endservent (void);		      \
extern enum nss_status _nss_ ## service ## _getservent_r		      \
		       (struct servent *serv, char *buffer, size_t buflen,    \
			int *errnop);					      \
extern enum nss_status _nss_ ## service ## _getservbyname_r		      \
		       (const char *name, const char *protocol,		      \
			struct servent *serv, char *buffer, size_t buflen,    \
			int *errnop);					      \
extern enum nss_status _nss_ ## service ## _getservbyport_r		      \
		       (int port, const char *protocol, struct servent *serv, \
			char *buffer, size_t buflen, int *errnop);	      \
extern enum nss_status _nss_ ## service ## _setnetgrent			      \
		       (const char *group, struct __netgrent *result);	      \
extern enum nss_status _nss_ ## service ## _endnetgrent			      \
		       (struct __netgrent *result);			      \
extern enum nss_status _nss_ ## service ## _getnetgrent_r		      \
		       (struct __netgrent *result, char *buffer,	      \
			size_t buflen, int *errnop);			      \
extern enum nss_status _nss_ ## service ## _setnetent (int stayopen);	      \
extern enum nss_status _nss_ ## service ## _endnetent (void);		      \
extern enum nss_status _nss_ ## service ## _getnetent_r			      \
			(struct netent *net, char *buffer, size_t buflen,     \
			 int *errnop, int *herrnop);			      \
extern enum nss_status _nss_ ## service ## _getnetbyname_r		      \
			(const char *name, struct netent *net, char *buffer,  \
			 size_t buflen, int *errnop, int *herrnop);	      \
extern enum nss_status _nss_ ## service ## _getnetbyaddr_r		      \
		       (uint32_t addr, int type, struct netent *net,	      \
			char *buffer, size_t buflen, int *errnop,	      \
			int *herrnop);

DECLARE_NSS_PROTOTYPES (compat)
DECLARE_NSS_PROTOTYPES (dns)
DECLARE_NSS_PROTOTYPES (files)
DECLARE_NSS_PROTOTYPES (hesiod)
DECLARE_NSS_PROTOTYPES (nis)
DECLARE_NSS_PROTOTYPES (nisplus)

#undef DECLARE_NSS_PROTOTYPES

#endif /* !_NETDB_H */
