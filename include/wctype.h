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
