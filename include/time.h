#ifndef _TIME_H
#if defined __need_time_t  || defined __need_clock_t || defined __need_timespec
# include <time/time.h>
#else
# include <time/time.h>

/* Now define the internal interfaces.  */
struct tm;

/* Defined in mktime.c.  */
extern const unsigned short int __mon_yday[2][13];

/* Defined in localtime.c.  */
extern struct tm _tmbuf;

/* Defined in tzset.c.  */
extern char *__tzstring (const char *string) __THROW;

/* Defined in tzset.c. */
extern size_t __tzname_cur_max;


extern int __use_tzfile;

extern void __tzfile_read (const char *file, size_t extra,
			   char **extrap) __THROW;
extern int __tzfile_compute (time_t timer, int use_localtime,
			     long int *leap_correct, int *leap_hit,
			     struct tm *tp) __THROW;
extern void __tzfile_default (const char *std, const char *dst,
			      long int stdoff, long int dstoff) __THROW;

/* Subroutine of `mktime'.  Return the `time_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `time_t' as performed
   by FUNC.  Keep track of next guess for time_t offset in *OFFSET.  */
extern time_t __mktime_internal (struct tm *__tp,
				 struct tm *(*__func) (const time_t *,
						       struct tm *),
				 time_t *__offset) __THROW;
extern struct tm *__localtime_r (__const time_t *__timer,
				 struct tm *__tp) __THROW;

extern struct tm *__gmtime_r (__const time_t *__restrict __timer,
			      struct tm *__restrict __tp) __THROW;

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
extern int __offtime (__const time_t *__timer,
		      long int __offset,
		      struct tm *__tp) __THROW;

extern char *__asctime_r (__const struct tm *__tp, char *__buf) __THROW;
extern void __tzset (void) __THROW;

/* Prototype for the internal function to get information based on TZ.  */
extern struct tm *__tz_convert (const time_t *timer, int use_localtime, struct tm *tp) __THROW;

/* Return the maximum length of a timezone name.
   This is what `sysconf (_SC_TZNAME_MAX)' does.  */
extern long int __tzname_max (void) __THROW;

extern int __nanosleep (__const struct timespec *__requested_time,
			struct timespec *__remaining) __THROW;
extern int __getdate_r (__const char *__string, struct tm *__resbufp) __THROW;
#endif
#endif
