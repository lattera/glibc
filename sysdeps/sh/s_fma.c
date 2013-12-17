#ifdef __SH_FPU_ANY__
# include <sysdeps/ieee754/dbl-64/s_fma.c>
#else
# include <soft-fp/fmadf4.c>
#endif
