#ifndef _WCTYPE_H

/* We try to get wint_t from <stddef.h>, but not all GCC versions define it
   there.  So define it ourselves if it remains undefined.  */
# define __need_wint_t
# include <stddef.h>
# ifndef _WINT_T
/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
#  define _WINT_T
typedef unsigned int wint_t;
# endif

/* Need to repeat these prototypes here, as wctype/wctype.h defines all
   these as macros and thus we couldn't add libc_hidden_proto.  */

extern int iswalpha (wint_t __wc);
extern int iswdigit (wint_t __wc);
extern int iswlower (wint_t __wc);
extern int iswspace (wint_t __wc);
extern int iswxdigit (wint_t __wc);
extern wint_t towlower (wint_t __wc);
extern wint_t towupper (wint_t __wc);

libc_hidden_proto (iswalpha)
libc_hidden_proto (iswdigit)
libc_hidden_proto (iswlower)
libc_hidden_proto (iswspace)
libc_hidden_proto (iswxdigit)
libc_hidden_proto (towlower)
libc_hidden_proto (towupper)

#include <wctype/wctype.h>

/* Internal interfaces.  */
extern int __iswalpha_l_internal (wint_t __wc, __locale_t __locale)
     attribute_hidden;
extern int __iswdigit_l_internal (wint_t __wc, __locale_t __locale)
     attribute_hidden;
extern int __iswspace_l_internal (wint_t __wc, __locale_t __locale)
     attribute_hidden;
extern int __iswxdigit_l_internal (wint_t __wc, __locale_t __locale)
     attribute_hidden;
extern int __iswspace (wint_t __wc);
extern int __iswctype (wint_t __wc, wctype_t __desc);
extern int __iswctype_internal (wint_t __wc, wctype_t __desc) attribute_hidden;
extern wctype_t __wctype (__const char *__property);
extern wint_t __towctrans (wint_t __wc, wctrans_t __desc);

libc_hidden_proto (__iswctype)
libc_hidden_proto (__iswalnum_l)
libc_hidden_proto (__iswalpha_l)
libc_hidden_proto (__iswblank_l)
libc_hidden_proto (__iswcntrl_l)
libc_hidden_proto (__iswdigit_l)
libc_hidden_proto (__iswlower_l)
libc_hidden_proto (__iswgraph_l)
libc_hidden_proto (__iswprint_l)
libc_hidden_proto (__iswpunct_l)
libc_hidden_proto (__iswspace_l)
libc_hidden_proto (__iswupper_l)
libc_hidden_proto (__iswxdigit_l)
libc_hidden_proto (__towlower_l)
libc_hidden_proto (__towupper_l)

#endif
