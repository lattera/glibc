#define __ieee754_pow __ieee754_pow_fma4
#define __exp1 __exp1_fma4
#define __slowpow __slowpow_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/e_pow.c>
