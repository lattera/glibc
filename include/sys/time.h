#ifndef _SYS_TIME_H
#include <time/sys/time.h>

#ifndef _ISOMAC
/* Now document the internal interfaces.  */
extern int __gettimeofday (struct timeval *__tv,
			   struct timezone *__tz);
libc_hidden_proto (__gettimeofday)
libc_hidden_proto (gettimeofday)
extern int __settimeofday (const struct timeval *__tv,
			   const struct timezone *__tz)
	attribute_hidden;
extern int __adjtime (const struct timeval *__delta,
		      struct timeval *__olddelta);
extern int __getitimer (enum __itimer_which __which,
			struct itimerval *__value);
extern int __setitimer (enum __itimer_which __which,
			const struct itimerval *__restrict __new,
			struct itimerval *__restrict __old)
	attribute_hidden;
extern int __utimes (const char *__file, const struct timeval __tvp[2])
	attribute_hidden;
extern int __futimes (int fd, const struct timeval tvp[2]) attribute_hidden;

#endif
#endif
