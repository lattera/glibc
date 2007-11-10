#define _IO_new_fsetpos64 __renamed__IO_new_fsetpos64
#define _IO_fsetpos64 __renamed__IO_fsetpos64

#include "../../libio/iofsetpos.c"

#undef _IO_new_fsetpos64
#undef _IO_fsetpos64

strong_alias (_IO_new_fsetpos, _IO_new_fsetpos64)
strong_alias (_IO_new_fsetpos64, __new_fsetpos64)
versioned_symbol (libc, __new_fsetpos64, fsetpos64, GLIBC_2_2);
versioned_symbol (libc, _IO_new_fsetpos64, _IO_fsetpos64, GLIBC_2_2);
