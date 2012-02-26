#ifndef _NET_IF_H

# include_next <net/if.h>

#ifndef _ISOMAC
libc_hidden_proto (if_nametoindex)
libc_hidden_proto (if_indextoname)
libc_hidden_proto (if_nameindex)
libc_hidden_proto (if_freenameindex)
#endif

#endif
