/* We have to irritate the compiler a bit.  */
#define __wcstoull_internal __wcstoull_internal_XXX
#define wcstoull wcstoull_XXX

#include <sysdeps/generic/wcstoul.c>

#undef __wcstoull_internal
#undef wcstoull
strong_alias (__wcstoul_internal, __wcstoull_internal)
weak_alias (__wcstoull_internal, wcstoull)
