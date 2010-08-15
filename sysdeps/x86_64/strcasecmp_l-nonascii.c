#include <string.h>

extern int __strcasecmp_l_nonascii (__const char *__s1, __const char *__s2,
				    __locale_t __loc);

#define __strcasecmp_l __strcasecmp_l_nonascii
#define USE_IN_EXTENDED_LOCALE_MODEL    1
#include <string/strcasecmp.c>
