#define tan __tan_avx
#define __branred __branred_avx
#define __dbl_mp __dbl_mp_avx
#define __mpranred __mpranred_avx
#define __mptan __mptan_avx
#define __sub __sub_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/s_tan.c>
