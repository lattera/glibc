#ifndef _SYS_CDEFS_H

#include <misc/sys/cdefs.h>

extern void __chk_fail (void) __attribute__ ((__noreturn__));
libc_hidden_proto (__chk_fail)
rtld_hidden_proto (__chk_fail)

#endif
