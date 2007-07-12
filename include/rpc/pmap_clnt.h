#ifndef _RPC_PMAP_CLNT_H
# include <sunrpc/rpc/pmap_clnt.h>

libc_hidden_proto (pmap_getport)
libc_hidden_proto (pmap_set)
libc_hidden_proto (pmap_unset)

/* Defined in pm_getport.c.  */
extern int __get_socket (struct sockaddr_in *saddr)
     attribute_hidden internal_function;

#endif
