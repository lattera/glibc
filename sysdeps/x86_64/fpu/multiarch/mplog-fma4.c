#define __mplog __mplog_fma4
#define __add __add_fma4
#define __mpexp __mpexp_fma4
#define __mul __mul_fma4
#define __sub __sub_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/mplog.c>
