#define __add __add_avx
#define __mul __mul_avx
#define __sub __sub_avx
#define __dbl_mp __dbl_mp_avx
#define __dvd __dvd_avx

#define NO___CPY 1
#define NO___MP_DBL 1
#define NO___ACR 1
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/mpa.c>
