#ifndef _SYS_RESOURCE_H
#include <resource/sys/resource.h>

#ifndef _ISOMAC
libc_hidden_proto (getpriority)
libc_hidden_proto (setpriority)
libc_hidden_proto (getrlimit64)

/* Now define the internal interfaces.  */
extern int __getrlimit (enum __rlimit_resource __resource,
			struct rlimit *__rlimits);
libc_hidden_proto (__getrlimit)
extern int __getrusage (enum __rusage_who __who, struct rusage *__usage)
	attribute_hidden;

extern int __setrlimit (enum __rlimit_resource __resource,
			const struct rlimit *__rlimits);
#endif
#endif
