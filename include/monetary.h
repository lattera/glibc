#include <stdlib/monetary.h>
#ifndef _ISOMAC
#include <stdarg.h>

extern ssize_t __vstrfmon_l (char *s, size_t maxsize, __locale_t loc,
			     const char *format, va_list ap);
#endif
