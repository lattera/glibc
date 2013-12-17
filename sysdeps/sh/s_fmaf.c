#ifdef __SH_FPU_ANY__
# include <sysdeps/ieee754/dbl-64/s_fmaf.c>
#else
# include <soft-fp/fmasf4.c>
#endif
