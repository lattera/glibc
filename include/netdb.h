#ifndef	_NETDB_H
#include <resolv/netdb.h>

/* Document internal interfaces.  */
extern int __gethostent_r (struct hostent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct hostent **__restrict __result,
			   int *__restrict __h_errnop) __THROW;

extern int __gethostbyaddr_r (__const char *__restrict __addr,
			      size_t __len, int __type,
			      struct hostent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct hostent **__restrict __result,
			      int *__restrict __h_errnop) __THROW;

extern int __gethostbyname_r (__const char *__restrict __name,
			      struct hostent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct hostent **__restrict __result,
			      int *__restrict __h_errnop) __THROW;

extern int __gethostbyname2_r (__const char *__restrict __name, int __af,
			       struct hostent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct hostent **__restrict __result,
			       int *__restrict __h_errnop) __THROW;

extern int __getnetent_r (struct netent *__restrict __result_buf,
			  char *__restrict __buf, size_t __buflen,
			  struct netent **__restrict __result,
			  int *__restrict __h_errnop) __THROW;

extern int __getnetbyaddr_r (unsigned long int __net, int __type,
			     struct netent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct netent **__restrict __result,
			     int *__restrict __h_errnop) __THROW;

extern int __getnetbyname_r (__const char *__restrict __name,
			     struct netent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct netent **__restrict __result,
			     int *__restrict __h_errnop) __THROW;

extern int __getservent_r (struct servent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct servent **__restrict __result) __THROW;

extern int __getservbyname_r (__const char *__restrict __name,
			      __const char *__restrict __proto,
			      struct servent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct servent **__restrict __result) __THROW;

extern int __getservbyport_r (int __port,
			      __const char *__restrict __proto,
			      struct servent *__restrict __result_buf,
			      char *__restrict __buf, size_t __buflen,
			      struct servent **__restrict __result) __THROW;

extern int __getprotoent_r (struct protoent *__restrict __result_buf,
			    char *__restrict __buf, size_t __buflen,
			    struct protoent **__restrict __result) __THROW;

extern int __getprotobyname_r (__const char *__restrict __name,
			       struct protoent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct protoent **__restrict __result) __THROW;

extern int __getprotobynumber_r (int __proto,
				 struct protoent *__restrict __res_buf,
				 char *__restrict __buf, size_t __buflen,
				 struct protoent **__restrict __result) __THROW;

extern int __getnetgrent_r (char **__restrict __hostp,
			    char **__restrict __userp,
			    char **__restrict __domainp,
			    char *__restrict __buffer, size_t __buflen) __THROW;
#endif /* !_NETDB_H */
