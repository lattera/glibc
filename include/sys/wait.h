#include <posix/sys/wait.h>

/* Now define the internal interfaces.  */
extern __pid_t __waitpid __P ((__pid_t __pid, int *__stat_loc,
			       int __options));
extern __pid_t __wait3 __P ((__WAIT_STATUS __stat_loc,
			     int __options, struct rusage * __usage));
extern __pid_t __wait4 __P ((__pid_t __pid, __WAIT_STATUS __stat_loc,
			     int __options, struct rusage *__usage));
