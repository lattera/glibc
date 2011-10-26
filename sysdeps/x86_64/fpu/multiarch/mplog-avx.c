#define __mplog __mplog_avx
#define __add __add_avx
#define __mpexp __mpexp_avx
#define __mul __mul_avx
#define __sub __sub_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/mplog.c>
