#ifndef _SYS_RESOURCE_H
#include <resource/sys/resource.h>

/* Now define the internal interfaces.  */
extern int __getrlimit (enum __rlimit_resource __resource,
			struct rlimit *__rlimits) __THROW;
extern int __getrusage (enum __rusage_who __who, struct rusage *__usage) __THROW;
#endif
