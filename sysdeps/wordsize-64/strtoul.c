/* We have to irritate the compiler a bit.  */
#define __strtoull_internal __strtoull_internal_XXX
#define strtoull strtoull_XXX

#include <sysdeps/generic/strtoul.c>

#undef __strtoull_internal
#undef strtoull
strong_alias (__strtoul_internal, __strtoull_internal)
weak_alias (__strtoull_internal, strtoull)
