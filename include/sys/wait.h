#ifndef _SYS_WAIT_H
#include <posix/sys/wait.h>

/* Now define the internal interfaces.  */
extern __pid_t __waitpid (__pid_t __pid, int *__stat_loc,
			  int __options) __THROW;
extern __pid_t __wait3 (__WAIT_STATUS __stat_loc,
			int __options, struct rusage * __usage) __THROW;
extern __pid_t __wait4 (__pid_t __pid, __WAIT_STATUS __stat_loc,
			int __options, struct rusage *__usage) __THROW;
#endif
