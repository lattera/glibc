#define _IO_new_fgetpos64 __renamed__IO_new_fgetpos64
#define _IO_fgetpos64 __renamed__IO_fgetpos64

#include "../../libio/iofgetpos.c"

#undef _IO_new_fgetpos64
#undef _IO_fgetpos64

strong_alias (_IO_new_fgetpos, _IO_new_fgetpos64)
strong_alias (_IO_new_fgetpos64, __new_fgetpos64)
versioned_symbol (libc, _IO_new_fgetpos64, _IO_fgetpos64, GLIBC_2_2);
versioned_symbol (libc, __new_fgetpos64, fgetpos64, GLIBC_2_2);
