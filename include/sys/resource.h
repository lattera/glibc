#ifndef _SYS_RESOURCE_H
#include <resource/sys/resource.h>

/* Now define the internal interfaces.  */
extern int __getrlimit __P ((enum __rlimit_resource __resource,
			     struct rlimit *__rlimits));
extern int __getrusage __P ((enum __rusage_who __who, struct rusage *__usage));
#endif
