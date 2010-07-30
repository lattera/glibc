#include <string.h>

#define __strcasecmp_l __strcasecmp_l_nonascii
#define USE_IN_EXTENDED_LOCALE_MODEL    1
#include <string/strcasecmp.c>
