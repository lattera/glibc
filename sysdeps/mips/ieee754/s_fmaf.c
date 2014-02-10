#ifdef __mips_hard_float
# include <sysdeps/ieee754/dbl-64/s_fmaf.c>
#else
# include <soft-fp/fmasf4.c>
#endif
