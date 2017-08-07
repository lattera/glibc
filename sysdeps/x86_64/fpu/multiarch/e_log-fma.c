#define __ieee754_log __ieee754_log_fma
#define __mplog __mplog_fma
#define __add __add_fma
#define __dbl_mp __dbl_mp_fma
#define __sub __sub_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/e_log.c>
