#ifndef _RPC_CLNT_H
#include <sunrpc/rpc/clnt.h>

/* Now define the internal interfaces.  */
extern int _openchild (const char *command, FILE **fto, FILE **ffrom);

extern CLIENT *clnt_create_internal (__const char *__host,
				     __const u_long __prog,
				     __const u_long __vers,
				     __const char *__prot) attribute_hidden;
extern CLIENT *clnttcp_create_internal (struct sockaddr_in *__raddr,
					u_long __prog, u_long __version,
					int *__sockp, u_int __sendsz,
					u_int __recvsz) attribute_hidden;
extern CLIENT *clntudp_create_internal (struct sockaddr_in *__raddr,
					u_long __program, u_long __version,
					struct timeval __wait_resend,
					int *__sockp) attribute_hidden;
extern CLIENT *clntudp_bufcreate_internal (struct sockaddr_in *__raddr,
					   u_long __program, u_long __version,
					   struct timeval __wait_resend,
					   int *__sockp, u_int __sendsz,
					   u_int __recvsz) attribute_hidden;
extern CLIENT *clntunix_create_internal (struct sockaddr_un *__raddr,
					 u_long __program, u_long __version,
					 int *__sockp, u_int __sendsz,
					 u_int __recvsz) attribute_hidden;

libc_hidden_proto (clnt_sperrno)
libc_hidden_proto (clnt_spcreateerror)
libc_hidden_proto (clnt_perror)
libc_hidden_proto (clnt_sperror)
libc_hidden_proto (_rpc_dtablesize)

#endif
