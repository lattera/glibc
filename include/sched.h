#ifndef _SCHED_H
#include <posix/sched.h>

/* Now define the internal interfaces.  */
extern int __sched_setparam __P ((__pid_t __pid,
				  __const struct sched_param *__param));
extern int __sched_getparam __P ((__pid_t __pid, struct sched_param *__param));
extern int __sched_setscheduler __P ((__pid_t __pid, int __policy,
				      __const struct sched_param *__param));
extern int __sched_getscheduler __P ((__pid_t __pid));
extern int __sched_yield __P ((void));
extern int __sched_get_priority_max __P ((int __algorithm));
extern int __sched_get_priority_min __P ((int __algorithm));
extern int __sched_rr_get_interval __P ((__pid_t __pid, struct timespec *__t));

/* This is Linux specific.  */
extern int __clone __P ((int (*__fn) (void *__arg), void *__child_stack,
			 int __flags, void *__arg));
#endif
