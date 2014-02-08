#ifdef __SOFTFP__
# include <soft-fp/fmasf4.c>
#else
# include <sysdeps/ieee754/dbl-64/s_fmaf.c>
#endif
