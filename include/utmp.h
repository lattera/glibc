#ifndef _UTMP_H
#include <login/utmp.h>

/* Now define the internal interfaces.  */
extern void __updwtmp (__const char *__wtmp_file,
		       __const struct utmp *__utmp);
extern int __utmpname (__const char *__file);
extern struct utmp *__getutent (void);
extern void __setutent (void);
extern void __endutent (void);
extern struct utmp *__getutid (__const struct utmp *__id);
extern struct utmp *__getutline (__const struct utmp *__line);
extern struct utmp *__pututline (__const struct utmp *__utmp_ptr);
extern int __getutent_r (struct utmp *__buffer, struct utmp **__result);
extern int __getutid_r (__const struct utmp *__id, struct utmp *__buffer,
			struct utmp **__result);
extern int __getutline_r (__const struct utmp *__line,
			  struct utmp *__buffer, struct utmp **__result);

libutil_hidden_proto (login_tty)

#endif
