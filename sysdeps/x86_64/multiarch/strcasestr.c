extern char *__strcasestr_sse42_nonascii (const unsigned char *s1,
					  const unsigned char *s2)
  attribute_hidden;

#define USE_AS_STRCASESTR
#define STRSTR_SSE42 __strcasestr_sse42
#include "strstr.c"
