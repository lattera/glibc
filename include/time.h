#ifndef _TIME_H
#if defined __need_time_t  || defined __need_clock_t || defined __need_timespec
# include <time/time.h>
#else
# include <time/time.h>
# include <xlocale.h>

extern __typeof (strftime_l) __strftime_l;
libc_hidden_proto (__strftime_l)
extern __typeof (strptime_l) __strptime_l;

libc_hidden_proto (time)
libc_hidden_proto (asctime)
libc_hidden_proto (mktime)
libc_hidden_proto (timelocal)
libc_hidden_proto (localtime)
libc_hidden_proto (strftime)
libc_hidden_proto (strptime)

/* Now define the internal interfaces.  */
struct tm;

/* Defined in mktime.c.  */
extern const unsigned short int __mon_yday[2][13] attribute_hidden;

/* Defined in localtime.c.  */
extern struct tm _tmbuf attribute_hidden;

/* Defined in tzset.c.  */
extern char *__tzstring (const char *string);

/* Defined in tzset.c. */
extern size_t __tzname_cur_max attribute_hidden;


extern int __use_tzfile attribute_hidden;

extern void __tzfile_read (const char *file, size_t extra,
			   char **extrap);
extern void __tzfile_compute (time_t timer, int use_localtime,
			      long int *leap_correct, int *leap_hit,
			      struct tm *tp);
extern void __tzfile_default (const char *std, const char *dst,
			      long int stdoff, long int dstoff);

/* Subroutine of `mktime'.  Return the `time_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `time_t' as performed
   by FUNC.  Keep track of next guess for time_t offset in *OFFSET.  */
extern time_t __mktime_internal (struct tm *__tp,
				 struct tm *(*__func) (const time_t *,
						       struct tm *),
				 time_t *__offset);
extern struct tm *__localtime_r (__const time_t *__timer,
				 struct tm *__tp) attribute_hidden;

extern struct tm *__gmtime_r (__const time_t *__restrict __timer,
			      struct tm *__restrict __tp);
libc_hidden_proto (__gmtime_r)

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
extern int __offtime (__const time_t *__timer,
		      long int __offset,
		      struct tm *__tp);

extern char *__asctime_r (__const struct tm *__tp, char *__buf);
extern void __tzset (void);

/* Prototype for the internal function to get information based on TZ.  */
extern struct tm *__tz_convert (const time_t *timer, int use_localtime, struct tm *tp);

/* Return the maximum length of a timezone name.
   This is what `sysconf (_SC_TZNAME_MAX)' does.  */
extern long int __tzname_max (void);

extern int __nanosleep (__const struct timespec *__requested_time,
			struct timespec *__remaining);
libc_hidden_proto(__nanosleep)
extern int __getdate_r (__const char *__string, struct tm *__resbufp);


/* Determine CLK_TCK value.  */
extern int __getclktck (void);


/* strptime support.  */
/* Status of lookup: do we use the locale data or the raw data?  */
enum ptime_locale_status { not, loc, raw };

extern char * __strptime_internal (const char *rp, const char *fmt,
				   struct tm *tm,
				   enum ptime_locale_status *decided,
				   int era_cnt, __locale_t loc)
     internal_function;

extern double __difftime (time_t time1, time_t time0);


/* Use in the clock_* functions.  Size of the field representing the
   actual clock ID.  */
#ifndef _ISOMAC
# define CLOCK_IDFIELD_SIZE	3
#endif
#endif
#endif
