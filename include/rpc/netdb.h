#ifndef _RPC_NETDB_H
#include <sunrpc/rpc/netdb.h>

extern int __getrpcbyname_r (__const char *__name, struct rpcent *__result_buf,
			     char *__buffer, size_t __buflen,
			     struct rpcent **__result);
extern int __old_getrpcbyname_r (__const char *__name,
				 struct rpcent *__result_buf,
				 char *__buffer, size_t __buflen,
				 struct rpcent **__result);

extern int __getrpcbynumber_r (int __number, struct rpcent *__result_buf,
			       char *__buffer, size_t __buflen,
			       struct rpcent **__result);
extern int __old_getrpcbynumber_r (int __number, struct rpcent *__result_buf,
				   char *__buffer, size_t __buflen,
				   struct rpcent **__result);

extern int __getrpcent_r (struct rpcent *__result_buf, char *__buffer,
			  size_t __buflen, struct rpcent **__result);
extern int __old_getrpcent_r (struct rpcent *__result_buf, char *__buffer,
			      size_t __buflen, struct rpcent **__result);

#endif
