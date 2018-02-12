#define __ieee754_pow __ieee754_pow_fma
#define __exp1 __exp1_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/e_pow.c>
