#ifndef	_NETDB_H
#include <resolv/netdb.h>

/* Document internal interfaces.  */
extern int __gethostent_r __P ((struct hostent *__restrict __result_buf,
				char *__restrict __buf, size_t __buflen,
				struct hostent **__restrict __result,
				int *__restrict __h_errnop));

extern int __gethostbyaddr_r __P ((__const char *__restrict __addr,
				   int __len, int __type,
				   struct hostent *__restrict __result_buf,
				   char *__restrict __buf, size_t __buflen,
				   struct hostent **__restrict __result,
				   int *__restrict __h_errnop));

extern int __gethostbyname_r __P ((__const char *__restrict __name,
				   struct hostent *__restrict __result_buf,
				   char *__restrict __buf, size_t __buflen,
				   struct hostent **__restrict __result,
				   int *__restrict __h_errnop));

extern int __gethostbyname2_r __P ((__const char *__restrict __name, int __af,
				    struct hostent *__restrict __result_buf,
				    char *__restrict __buf, size_t __buflen,
				    struct hostent **__restrict __result,
				    int *__restrict __h_errnop));

extern int __getnetent_r __P ((struct netent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct netent **__restrict __result,
			       int *__restrict __h_errnop));

extern int __getnetbyaddr_r __P ((unsigned long int __net, int __type,
				  struct netent *__restrict __result_buf,
				  char *__restrict __buf, size_t __buflen,
				  struct netent **__restrict __result,
				  int *__restrict __h_errnop));

extern int __getnetbyname_r __P ((__const char *__restrict __name,
				  struct netent *__restrict __result_buf,
				  char *__restrict __buf, size_t __buflen,
				  struct netent **__restrict __result,
				  int *__restrict __h_errnop));

extern int __getservent_r __P ((struct servent *__restrict __result_buf,
				char *__restrict __buf, size_t __buflen,
				struct servent **__restrict __result));

extern int __getservbyname_r __P ((__const char *__restrict __name,
				   __const char *__restrict __proto,
				   struct servent *__restrict __result_buf,
				   char *__restrict __buf, size_t __buflen,
				   struct servent **__restrict __result));

extern int __getservbyport_r __P ((int __port,
				   __const char *__restrict __proto,
				   struct servent *__restrict __result_buf,
				   char *__restrict __buf, size_t __buflen,
				   struct servent **__restrict __result));

extern int __getprotoent_r __P ((struct protoent *__restrict __result_buf,
				 char *__restrict __buf, size_t __buflen,
				 struct protoent **__restrict __result));

extern int __getprotobyname_r __P ((__const char *__restrict __name,
				    struct protoent *__restrict __result_buf,
				    char *__restrict __buf, size_t __buflen,
				    struct protoent **__restrict __result));

extern int __getprotobynumber_r __P ((int __proto,
				      struct protoent *__restrict __res_buf,
				      char *__restrict __buf, size_t __buflen,
				      struct protoent **__restrict __result));

extern int __getnetgrent_r __P ((char **__restrict __hostp,
				 char **__restrict __userp,
				 char **__restrict __domainp,
				 char *__restrict __buffer, size_t __buflen));
#endif /* !_NETDB_H */
