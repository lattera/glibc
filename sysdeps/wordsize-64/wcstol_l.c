/* We have to irritate the compiler a bit.  */
#define ____wcstoll_l_internal ____wcstoll_l_internal_XXX
#define __wcstoll_l __wcstoll_l_XXX

#include <sysdeps/generic/wcstol_l.c>

#undef ____wcstoll_l_internal
#undef __wcstoll_l
strong_alias (____wcstol_l_internal, ____wcstoll_l_internal)
weak_alias (__wcstol_l, __wcstoll_l)
