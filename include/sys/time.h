#ifndef _SYS_TIME_H
#include <time/sys/time.h>

/* Now document the internal interfaces.  */
extern int __gettimeofday __P ((struct timeval *__tv,
				struct timezone *__tz));
extern int __settimeofday __P ((__const struct timeval *__tv,
				__const struct timezone *__tz));
extern int __adjtime __P ((__const struct timeval *__delta,
			   struct timeval *__olddelta));
extern int __getitimer __P ((enum __itimer_which __which,
			     struct itimerval *__value));
extern int __setitimer __P ((enum __itimer_which __which,
			     __const struct itimerval *__new,
			     struct itimerval *__old));
extern int __utimes __P ((__const char *__file, struct timeval __tvp[2]));
#endif
