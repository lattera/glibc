#ifndef _MACH_MIG_SUPPORT_H
#include_next <mach/mig_support.h>
#ifndef _ISOMAC
libc_hidden_proto (__mig_get_reply_port)
libc_hidden_proto (__mig_dealloc_reply_port)
libc_hidden_proto (__mig_init)
#endif
#endif
