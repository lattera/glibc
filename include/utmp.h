#ifndef _UTMP_H
#include <login/utmp.h>

/* Now define the internal interfaces.  */
extern void __updwtmp (__const char *__wtmp_file,
		       __const struct utmp *__utmp) __THROW;
extern int __utmpname (__const char *__file) __THROW;
extern struct utmp *__getutent (void) __THROW;
extern void __setutent (void) __THROW;
extern void __endutent (void) __THROW;
extern struct utmp *__getutid (__const struct utmp *__id) __THROW;
extern struct utmp *__getutline (__const struct utmp *__line) __THROW;
extern struct utmp *__pututline (__const struct utmp *__utmp_ptr) __THROW;
extern int __getutent_r (struct utmp *__buffer, struct utmp **__result) __THROW;
extern int __getutid_r (__const struct utmp *__id, struct utmp *__buffer,
			struct utmp **__result) __THROW;
extern int __getutline_r (__const struct utmp *__line,
			  struct utmp *__buffer, struct utmp **__result) __THROW;
#endif
