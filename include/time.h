#ifndef _TIME_H
#if defined __need_time_t  || defined __need_clock_t || defined __need_timespec
# include <time/time.h>
#else
# include <time/time.h>

/* Now define the internal interfaces.  */
struct tm;

/* Subroutine of `mktime'.  Return the `time_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `time_t' as performed
   by FUNC.  Keep track of next guess for time_t offset in *OFFSET.  */
extern time_t __mktime_internal __P ((struct tm *__tp,
				      struct tm *(*__func) (const time_t *,
							    struct tm *),
				      time_t *__offset));
extern struct tm *__localtime_r __P ((__const time_t *__timer,
				      struct tm *__tp));

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
extern int __offtime __P ((__const time_t *__timer,
			   long int __offset,
			   struct tm *__tp));

extern char *__asctime_r __P ((__const struct tm *__tp, char *__buf));
extern void __tzset __P ((void));

/* Return the maximum length of a timezone name.
   This is what `sysconf (_SC_TZNAME_MAX)' does.  */
extern long int __tzname_max __P ((void));

extern int __nanosleep __P ((__const struct timespec *__requested_time,
			     struct timespec *__remaining));
extern int __getdate_r __P ((__const char *__string, struct tm *__resbufp));
#endif
#endif
