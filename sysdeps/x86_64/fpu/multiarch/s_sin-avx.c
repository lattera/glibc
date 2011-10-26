#define __cos __cos_avx
#define __sin __sin_avx
#define __branred __branred_avx
#define __docos __docos_avx
#define __dubsin __dubsin_avx
#define __mpcos __mpcos_avx
#define __mpcos1 __mpcos1_avx
#define __mpsin __mpsin_avx
#define __mpsin1 __mpsin1_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/s_sin.c>
