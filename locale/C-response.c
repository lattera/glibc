#include <ansidecl.h>
#include <localeinfo.h>
#include <stddef.h>


CONST struct response_info __response_C =
  {
    (char *) "[yY][[:alpha:]]",
    (char *) "[nN][[:alpha:]]"
  };

CONST struct response_info *_response_info = &__response_C;
