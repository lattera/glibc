/* We have to irritate the compiler a bit.  */
#define __wcstoll_internal __wcstoll_internal_XXX
#define wcstoll wcstoll_XXX
#define wcstoq wcstoq_XXX

#include <sysdeps/generic/wcstol.c>

#undef __wcstoll_internal
#undef wcstoll
#undef wcstoq
strong_alias (__wcstol_internal, __wcstoll_internal)
libc_hidden_def (__wcstoll_internal)
weak_alias (wcstol, wcstoll)
libc_hidden_weak (wcstoll)
weak_alias (wcstol, wcstoq)
libc_hidden_weak (wcstoq)
