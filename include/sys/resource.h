#ifndef _SYS_RESOURCE_H
#include <resource/sys/resource.h>

/* Now define the internal interfaces.  */
extern int __getrlimit (enum __rlimit_resource __resource,
			struct rlimit *__rlimits);
extern int __getrusage (enum __rusage_who __who, struct rusage *__usage);

extern int __setrlimit (enum __rlimit_resource __resource,
			const struct rlimit *__rlimits);
#endif
