#ifndef _SYS_RESOURCE_H
#include <resource/sys/resource.h>

#ifndef _ISOMAC
/* Prototypes repeated instead of using __typeof because
   sys/resource.h is included in C++ tests, and declaring functions
   with __typeof and __THROW doesn't work for C++.  */
extern int __getpriority (__priority_which_t __which, id_t __who) __THROW;
libc_hidden_proto (__getpriority)
extern int __setpriority (__priority_which_t __which, id_t __who, int __prio)
     __THROW;
libc_hidden_proto (__setpriority)
libc_hidden_proto (getrlimit64)
extern __typeof (getrlimit64) __getrlimit64;
libc_hidden_proto (__getrlimit64);

/* Now define the internal interfaces.  */
extern int __getrlimit (enum __rlimit_resource __resource,
			struct rlimit *__rlimits);
libc_hidden_proto (__getrlimit)
extern int __getrusage (enum __rusage_who __who, struct rusage *__usage)
	attribute_hidden;

extern int __setrlimit (enum __rlimit_resource __resource,
			const struct rlimit *__rlimits);
libc_hidden_proto (__setrlimit);
#endif
#endif
