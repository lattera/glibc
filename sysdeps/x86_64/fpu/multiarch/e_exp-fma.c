#define __ieee754_exp __ieee754_exp_fma
#define __exp1 __exp1_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/e_exp.c>
