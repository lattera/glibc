#include <login/utmp.h>

/* Now define the internal interfaces.  */
extern void __updwtmp __P ((__const char *__wtmp_file,
			    __const struct utmp *__utmp));
extern int __utmpname __P ((__const char *__file));
extern struct utmp *__getutent __P ((void));
extern void __setutent __P ((void));
extern void __endutent __P ((void));
extern struct utmp *__getutid __P ((__const struct utmp *__id));
extern struct utmp *__getutline __P ((__const struct utmp *__line));
extern struct utmp *__pututline __P ((__const struct utmp *__utmp_ptr));
extern int __getutent_r __P ((struct utmp *__buffer, struct utmp **__result));
extern int __getutid_r __P ((__const struct utmp *__id, struct utmp *__buffer,
			     struct utmp **__result));
extern int __getutline_r __P ((__const struct utmp *__line,
			       struct utmp *__buffer, struct utmp **__result));
