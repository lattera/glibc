#define __mptan __mptan_avx
#define __c32 __c32_avx
#define __dvd __dvd_avx
#define __mpranred __mpranred_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/mptan.c>
