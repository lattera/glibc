#ifndef	_LOCALE_H
#include <locale/locale.h>

/* This has to be changed whenever a new locale is defined.  */
#define __LC_LAST	13

/* Locale object for C locale.  */
extern struct __locale_struct _nl_C_locobj;

/* Now define the internal interfaces.  */
extern struct lconv *__localeconv (void);

#endif
