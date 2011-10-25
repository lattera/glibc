#define __ieee754_log __ieee754_log_fma4
#define __mplog __mplog_fma4
#define __add __add_fma4
#define __dbl_mp __dbl_mp_fma4
#define __sub __sub_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/e_log.c>
