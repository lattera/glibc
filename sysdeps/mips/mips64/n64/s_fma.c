/* MIPS long double is implemented in software by fp-bit (as of GCC
   4.7) without support for exceptions or rounding modes, so the fma
   implementation in terms of long double is slow and will not produce
   correctly rounding results.  */

#include <sysdeps/ieee754/dbl-64/s_fma.c>
