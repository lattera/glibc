#ifndef _SCHED_H
#include <posix/sched.h>

/* Now define the internal interfaces.  */
extern int __sched_setparam (__pid_t __pid,
			     __const struct sched_param *__param) __THROW;
extern int __sched_getparam (__pid_t __pid, struct sched_param *__param) __THROW;
extern int __sched_setscheduler (__pid_t __pid, int __policy,
				 __const struct sched_param *__param) __THROW;
extern int __sched_getscheduler (__pid_t __pid) __THROW;
extern int __sched_yield (void) __THROW;
extern int __sched_get_priority_max (int __algorithm) __THROW;
extern int __sched_get_priority_min (int __algorithm) __THROW;
extern int __sched_rr_get_interval (__pid_t __pid, struct timespec *__t) __THROW;

/* This is Linux specific.  */
extern int __clone (int (*__fn) (void *__arg), void *__child_stack,
		    int __flags, void *__arg) __THROW;
#endif
