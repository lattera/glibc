#ifndef	_NETDB_H
#include <resolv/netdb.h>

/* Document internal interfaces.  */
extern int __gethostent_r (struct hostent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct hostent **__restrict __result,
			   int *__restrict __h_errnop);

extern int __gethostbyaddr_r (__const void *__restrict __addr,
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

extern int __gethostbyname2_r (__const char *__restrict __name, int __af,
			       struct hostent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct hostent **__restrict __result,
			       int *__restrict __h_errnop);

extern int __getnetent_r (struct netent *__restrict __result_buf,
			  char *__restrict __buf, size_t __buflen,
			  struct netent **__restrict __result,
			  int *__restrict __h_errnop);

extern int __getnetbyaddr_r (uint32_t __net, int __type,
			     struct netent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct netent **__restrict __result,
			     int *__restrict __h_errnop);

extern int __getnetbyname_r (__const char *__restrict __name,
			     struct netent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct netent **__restrict __result,
			     int *__restrict __h_errnop);

extern int __getservent_r (struct servent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct servent **__restrict __result);

extern int __getservbyname_r (__const char *__restrict __name,
			      __const char *__restrict __proto,
			      struct servent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct servent **__restrict __result);

extern int __getservbyport_r (int __port,
			      __const char *__restrict __proto,
			      struct servent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct servent **__restrict __result);

extern int __getprotoent_r (struct protoent *__restrict __result_buf,
			    char *__restrict __buf, size_t __buflen,
			    struct protoent **__restrict __result);

extern int __getprotobyname_r (__const char *__restrict __name,
			       struct protoent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct protoent **__restrict __result);

extern int __getprotobynumber_r (int __proto,
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

/* Return entry from host data base which address match ADDR with
   length LEN and type TYPE in newly allocated buffer.  */
extern struct hostent *getipnodebyaddr (__const void *__addr, socklen_t __len,
					int __type, int *__error_num) __THROW;

/* Return entry from host data base for host with NAME and newly allocated
   buffer.  FLAGS is some combination of the following AI_* values.  */
extern struct hostent *getipnodebyname (__const char *__name, int __type,
					int __flags, int *__error_num) __THROW;

#define AI_V4MAPPED	0x0008  /* IPv4-mapped addresses are acceptable.  */
#define AI_ALL		0x0010  /* Return both IPv4 and IPv6 addresses.  */
#define AI_ADDRCONFIG	0x0020  /* Use configuration of this host to choose
                                  returned address type.  */
#define AI_DEFAULT    (AI_V4MAPPED | AI_ADDRCONFIG)

/* Free structure returned by previous `getipnodebyaddr' or `getipnodebyname'
   call.  */
extern void freehostent (struct hostent *__ptr) __THROW;

#endif /* !_NETDB_H */
