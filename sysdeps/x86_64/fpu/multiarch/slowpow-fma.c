#define __slowpow __slowpow_fma
#define __add __add_fma
#define __dbl_mp __dbl_mp_fma
#define __mpexp __mpexp_fma
#define __mplog __mplog_fma
#define __mul __mul_fma
#define __sub __sub_fma
#define __halfulp __halfulp_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/slowpow.c>
