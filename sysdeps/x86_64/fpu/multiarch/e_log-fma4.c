#define __ieee754_log __ieee754_log_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/e_log.c>
