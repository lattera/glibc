#include <ansidecl.h>
#include <localeinfo.h>
#include <stddef.h>


CONST struct monetary_info __monetary_C =
  {
    (char *) "", (char *) "",
    (char *) "", (char *) "",
    (char *) "",
    (char *) "", (char *) "",
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX
  };

CONST struct monetary_info *_monetary_info = &__monetary_C;
