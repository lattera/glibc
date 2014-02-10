#ifdef __mips_hard_float
# include <sysdeps/ieee754/dbl-64/s_fma.c>
#else
# include <soft-fp/fmadf4.c>
#endif
