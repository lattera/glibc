#ifndef _SYS_TIME_H
#include <time/sys/time.h>

/* Now document the internal interfaces.  */
extern int __gettimeofday (struct timeval *__tv,
			   struct timezone *__tz);
extern int __gettimeofday_internal (struct timeval *__tv,
				    struct timezone *__tz)
	attribute_hidden;
extern int __settimeofday (__const struct timeval *__tv,
			   __const struct timezone *__tz)
	attribute_hidden;
extern int __adjtime (__const struct timeval *__delta,
		      struct timeval *__olddelta);
extern int __getitimer (enum __itimer_which __which,
			struct itimerval *__value);
extern int __setitimer (enum __itimer_which __which,
			__const struct itimerval *__restrict __new,
			struct itimerval *__restrict __old)
	attribute_hidden;
extern int __utimes (__const char *__file, const struct timeval __tvp[2])
	attribute_hidden;

#ifndef NOT_IN_libc
# define __gettimeofday(tv, tz) INTUSE(__gettimeofday) (tv, tz)
#endif

#endif
