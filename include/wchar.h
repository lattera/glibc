#include <wcsmbs/wchar.h>

/* Now define the internal interfaces.  */
extern int __wcscasecmp __P ((__const wchar_t *__s1, __const wchar_t *__s2));
extern int __wcsncasecmp __P ((__const wchar_t *__s1, __const wchar_t *__s2,
                               size_t __n));
extern size_t __wcsnlen __P ((__const wchar_t *__s, size_t __maxlen));
extern wint_t __btowc __P ((int __c));
extern int __mbsinit __P ((__const mbstate_t *__ps));
extern size_t __mbrtowc __P ((wchar_t *__restrict __pwc,
			      __const char *__restrict __s, size_t __n,
			      mbstate_t *__restrict __p));
extern size_t __wcrtomb __P ((char *__restrict __s, wchar_t __wc,
			      mbstate_t *__restrict __ps));
extern size_t __mbsrtowcs __P ((wchar_t *__restrict __dst,
				__const char **__restrict __src,
				size_t __len, mbstate_t *__restrict __ps));
extern size_t __wcsrtombs __P ((char *__restrict __dst,
				__const wchar_t **__restrict __src,
				size_t __len, mbstate_t *__restrict __ps));
extern size_t __mbsnrtowcs __P ((wchar_t *__restrict __dst,
				 __const char **__restrict __src, size_t __nmc,
				 size_t __len, mbstate_t *__restrict __ps));
extern size_t __wcsnrtombs __P ((char *__restrict __dst,
				 __const wchar_t **__restrict __src,
				 size_t __nwc, size_t __len,
				 mbstate_t *__restrict __ps));
extern wchar_t *__wcpcpy __P ((wchar_t *__dest, __const wchar_t *__src));
extern wchar_t *__wcpncpy __P ((wchar_t *__dest, __const wchar_t *__src,
				size_t __n));
