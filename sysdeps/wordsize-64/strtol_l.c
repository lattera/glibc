/* We have to irritate the compiler a bit.  */
#define ____strtoll_l_internal ____strtoll_l_internal_XXX
#define __strtoll_l __strtoll_l_XXX

#include <sysdeps/generic/strtol_l.c>

#undef ____strtoll_l_internal
#undef __strtoll_l
strong_alias (____strtol_l_internal, ____strtoll_l_internal)
weak_alias (__strtol_l, __strtoll_l)
