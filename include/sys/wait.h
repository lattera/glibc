#ifndef _SYS_WAIT_H
#include <posix/sys/wait.h>

/* Now define the internal interfaces.  */
extern __pid_t __waitpid (__pid_t __pid, int *__stat_loc,
			  int __options);
libc_hidden_proto (__waitpid)
extern int __waitid (idtype_t idtype, id_t id, siginfo_t *infop, int options);

extern __pid_t __libc_waitpid (pid_t __pid, int *__stat_loc, int __options);
extern __pid_t __libc_wait (int *__stat_loc);
extern __pid_t __wait (__WAIT_STATUS __stat_loc);
extern __pid_t __wait3 (__WAIT_STATUS __stat_loc,
			int __options, struct rusage * __usage);
extern __pid_t __wait4 (__pid_t __pid, __WAIT_STATUS __stat_loc,
			int __options, struct rusage *__usage)
			attribute_hidden;
#endif
