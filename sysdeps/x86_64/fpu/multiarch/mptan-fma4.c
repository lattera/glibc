#define __mptan __mptan_fma4
#define __c32 __c32_fma4
#define __dvd __dvd_fma4
#define __mpranred __mpranred_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/mptan.c>
