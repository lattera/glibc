#include <complex.h>
#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#define __clog10l __clog10l_internal
#include <math/s_clog10l.c>
#undef __clog10l
strong_alias (__clog10l_internal, __clog10l__internal)
long_double_symbol (libm, __clog10l_internal, __clog10l);
long_double_symbol (libm, __clog10l__internal, clog10l);
