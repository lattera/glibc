#ifndef _WCTYPE_H

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

#ifndef NOT_IN_libc
# define __iswalpha_l(wc, loc) INTUSE(__iswalpha_l) (wc, loc)
# define __iswctype(wc, desc) INTUSE(__iswctype) (wc, desc)
# define __iswdigit_l(wc, loc) INTUSE(__iswdigit_l) (wc, loc)
# define __iswspace_l(wc, loc) INTUSE(__iswspace_l) (wc, loc)
# define __iswxdigit_l(wc, loc) INTUSE(__iswxdigit_l) (wc, loc)
#endif

#endif
