#ifndef _LIBINTL_H
#include <intl/libintl.h>
#include <locale.h>

/* Now define the internal interfaces.  */
extern char *__gettext (__const char *__msgid);
extern char *__dgettext (__const char *__domainname,
			 __const char *__msgid);
extern char *__dcgettext (__const char *__domainname,
			  __const char *__msgid, int __category);
extern char *__textdomain (__const char *__domainname);
extern char *__bindtextdomain (__const char *__domainname,
			       __const char *__dirname);
extern char *__bind_textdomain_codeset (__const char *__domainname,
					__const char *__codeset);
extern const char _libc_intl_domainname[];

/* Define the macros `_' and `N_' for conveniently marking translatable
   strings in the libc source code.  We have to make sure we get the
   correct definitions so we undefine the macros first.  */

# undef N_
# define N_(msgid)	msgid

# undef _
/* This is defined as an optimizing macro, so use it.  */
# define _(msgid) __dcgettext (_libc_intl_domainname, (msgid), LC_MESSAGES)

#endif
