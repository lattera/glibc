#include <ansidecl.h>
#include <localeinfo.h>
#include <stddef.h>


extern CONST struct ctype_ctype_info __ctype_ctype_C;
extern CONST struct ctype_mbchar_info __ctype_mbchar_C;
CONST struct ctype_info __ctype_C =
  {
    (struct ctype_ctype_info*)&__ctype_ctype_C,
    (struct ctype_mbchar_info*) &__ctype_mbchar_C
  };

CONST struct ctype_info *_ctype_info = &__ctype_C;
