#define __ieee754_exp __ieee754_exp_avx
#define __exp1 __exp1_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/e_exp.c>
