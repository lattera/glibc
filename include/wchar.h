#ifndef _WCHAR_H
#include <wcsmbs/wchar.h>

# ifdef _WCHAR_H
/* Now define the internal interfaces.  */
extern int __wcscasecmp __P ((__const wchar_t *__s1, __const wchar_t *__s2));
extern int __wcsncasecmp __P ((__const wchar_t *__s1, __const wchar_t *__s2,
                               size_t __n));
extern size_t __wcsnlen __P ((__const wchar_t *__s, size_t __maxlen));
extern wint_t __btowc __P ((int __c));
extern int __mbsinit __P ((__const __mbstate_t *__ps));
extern size_t __mbrtowc __P ((wchar_t *__restrict __pwc,
			      __const char *__restrict __s, size_t __n,
			      __mbstate_t *__restrict __p));
extern size_t __wcrtomb __P ((char *__restrict __s, wchar_t __wc,
			      __mbstate_t *__restrict __ps));
extern size_t __mbsrtowcs __P ((wchar_t *__restrict __dst,
				__const char **__restrict __src,
				size_t __len, __mbstate_t *__restrict __ps));
extern size_t __wcsrtombs __P ((char *__restrict __dst,
				__const wchar_t **__restrict __src,
				size_t __len, __mbstate_t *__restrict __ps));
extern size_t __mbsnrtowcs __P ((wchar_t *__restrict __dst,
				 __const char **__restrict __src, size_t __nmc,
				 size_t __len, __mbstate_t *__restrict __ps));
extern size_t __wcsnrtombs __P ((char *__restrict __dst,
				 __const wchar_t **__restrict __src,
				 size_t __nwc, size_t __len,
				 __mbstate_t *__restrict __ps));
extern wchar_t *__wcpcpy __P ((wchar_t *__dest, __const wchar_t *__src));
extern wchar_t *__wcpncpy __P ((wchar_t *__dest, __const wchar_t *__src,
				size_t __n));
extern wchar_t *__wmemcpy __P ((wchar_t *__s1, __const wchar_t *s2,
				size_t __n));
extern wchar_t *__wmempcpy __P ((wchar_t *__restrict __s1,
				 __const wchar_t *__restrict __s2,
				 size_t __n));
extern wchar_t *__wmemmove __P ((wchar_t *__s1, __const wchar_t *__s2,
				 size_t __n));
extern wchar_t *__wcschrnul __P ((__const wchar_t *__s, wchar_t __wc));

extern int __vfwscanf __P ((FILE *__restrict __s,
			    __const wchar_t *__restrict __format,
			    va_list __arg))
     /* __attribute__ ((__format__ (__wscanf__, 2, 0))) */;
# endif
#endif
