#ifndef _SYS_CDEFS_H

#include <misc/sys/cdefs.h>

/* The compiler will optimize based on the knowledge the parameter is
   not NULL.  This will omit tests.  A robust implementation cannot allow
   this so when compiling glibc itself we ignore this attribute.  */
#undef __nonnull
#define __nonnull(params)

extern void __chk_fail (void) __attribute__ ((__noreturn__));
libc_hidden_proto (__chk_fail)
rtld_hidden_proto (__chk_fail)


#if __GNUC_PREREQ (4,3)
# define __attribute_alloc_size(...) __attribute__ ((alloc_size (__VA_ARGS__)))
#else
# define __attribute_alloc_size(...)
#endif

#endif
