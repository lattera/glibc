#define __mpexp __mpexp_avx
#define __add __add_avx
#define __dbl_mp __dbl_mp_avx
#define __dvd __dvd_avx
#define __mul __mul_avx
#define AVOID_MPEXP_H 1
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/mpexp.c>
