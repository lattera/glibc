#define __mptan __mptan_fma
#define __c32 __c32_fma
#define __dvd __dvd_fma
#define __mpranred __mpranred_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/mptan.c>
