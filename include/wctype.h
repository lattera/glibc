#ifndef _WCTYPE_H

#ifndef _ISOMAC
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
extern int iswalnum (wint_t __wc);
extern int iswdigit (wint_t __wc);
extern int iswlower (wint_t __wc);
extern int iswspace (wint_t __wc);
extern int iswxdigit (wint_t __wc);
extern wint_t towlower (wint_t __wc);
extern wint_t towupper (wint_t __wc);

libc_hidden_proto (iswalpha)
libc_hidden_proto (iswalnum)
libc_hidden_proto (iswdigit)
libc_hidden_proto (iswlower)
libc_hidden_proto (iswspace)
libc_hidden_proto (iswxdigit)
libc_hidden_proto (towlower)
libc_hidden_proto (towupper)
#endif

#include <wctype/wctype.h>

#ifndef _ISOMAC
/* Internal interfaces.  */
extern int __iswspace (wint_t __wc);
extern int __iswctype (wint_t __wc, wctype_t __desc);
extern wctype_t __wctype (const char *__property);
extern wctrans_t __wctrans (const char *__property);
extern wint_t __towctrans (wint_t __wc, wctrans_t __desc);

extern __typeof (iswalnum_l) __iswalnum_l;
extern __typeof (iswalpha_l) __iswalpha_l;
extern __typeof (iswblank_l) __iswblank_l;
extern __typeof (iswcntrl_l) __iswcntrl_l;
extern __typeof (iswdigit_l) __iswdigit_l;
extern __typeof (iswlower_l) __iswlower_l;
extern __typeof (iswgraph_l) __iswgraph_l;
extern __typeof (iswprint_l) __iswprint_l;
extern __typeof (iswpunct_l) __iswpunct_l;
extern __typeof (iswspace_l) __iswspace_l;
extern __typeof (iswupper_l) __iswupper_l;
extern __typeof (iswxdigit_l) __iswxdigit_l;
extern __typeof (towlower_l) __towlower_l;
extern __typeof (towupper_l) __towupper_l;

libc_hidden_proto (__towctrans)
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

/* The spec says that isdigit must only match the decimal digits.  We
   can check this without a memory access.  */
# if IS_IN (libc)
#  undef iswdigit
#  define iswdigit(c) ({ wint_t __c = (c); __c >= L'0' && __c <= L'9'; })
#  undef iswdigit_l
#  define iswdigit_l(c, l) ({ wint_t __c = (c); __c >= L'0' && __c <= L'9'; })
#  undef __iswdigit_l
#  define __iswdigit_l(c, l) ({ wint_t __c = (c); __c >= L'0' && __c <= L'9'; })
# endif

#endif
#endif
