#ifndef _LIBINTL_H
#include <intl/libintl.h>
#include <locale.h>

/* Now define the internal interfaces.  */
extern char *__gettext (const char *__msgid)
     __attribute_format_arg__ (1);
extern char *__dgettext (const char *__domainname,
			 const char *__msgid)
     __attribute_format_arg__ (2);
extern char *__dcgettext (const char *__domainname,
			  const char *__msgid, int __category)
     __attribute_format_arg__ (2);
libc_hidden_proto (__dcgettext)

extern char *__ngettext (const char *__msgid1, const char *__msgid2,
			 unsigned long int __n)
     __attribute_format_arg__ (1) __attribute_format_arg__ (2);
extern char *__dngettext (const char *__domainname,
			  const char *__msgid1, const char *__msgid2,
			  unsigned long int __n)
     __attribute_format_arg__ (2) __attribute_format_arg__ (3);
extern char *__dcngettext (const char *__domainname,
			   const char *__msgid1, const char *__msgid2,
			   unsigned long int __n, int __category)
     __attribute_format_arg__ (2) __attribute_format_arg__ (3);

extern char *__textdomain (const char *__domainname);
extern char *__bindtextdomain (const char *__domainname,
			       const char *__dirname);
extern char *__bind_textdomain_codeset (const char *__domainname,
					const char *__codeset);

extern const char _libc_intl_domainname[];
libc_hidden_proto (_libc_intl_domainname)

/* Define the macros `_' and `N_' for conveniently marking translatable
   strings in the libc source code.  We have to make sure we get the
   correct definitions so we undefine the macros first.  */

# undef N_
# define N_(msgid)	msgid

# undef _
/* This is defined as an optimizing macro, so use it.  */
# define _(msgid) \
  __dcgettext (_libc_intl_domainname, msgid, LC_MESSAGES)

#endif
