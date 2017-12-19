#define __ieee754_exp __ieee754_exp_fma4
#define __exp1 __exp1_fma4
#define __slowexp __slowexp_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/e_exp.c>
