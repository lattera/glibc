#include <stdlib.h>
#include <soft-fp.h>
#include <quad.h>

/* Helpers for the Ots functions which receive long double arguments
   in two integer registers, and return values in $16+$17.  */

#undef _FP_UNPACK_RAW_2
#define _FP_UNPACK_RAW_2(fs, X, val)                    \
  do {                                                  \
    union _FP_UNION_##fs _flo;				\
    _flo.longs.a = val##l;				\
    _flo.longs.b = val##h;				\
    X##_f0 = _flo.bits.frac0;				\
    X##_f1 = _flo.bits.frac1;				\
    X##_e  = _flo.bits.exp;				\
    X##_s  = _flo.bits.sign;				\
  } while (0)

#undef _FP_PACK_RAW_2
#define _FP_PACK_RAW_2(fs, val, X)                      \
  do {                                                  \
    union _FP_UNION_##fs _flo;				\
    _flo.bits.frac0 = X##_f0;				\
    _flo.bits.frac1 = X##_f1;				\
    _flo.bits.exp   = X##_e;				\
    _flo.bits.sign  = X##_s;				\
    val##l = _flo.longs.a;				\
    val##h = _flo.longs.b;				\
  } while (0)

#define FP_DECL_RETURN(X) \
  long X##l, X##h

/* ??? We don't have a real way to tell the compiler that we're wanting
   to return values in $16+$17.  Instead use a volatile asm to make sure
   that the values are live, and just hope that nothing kills the values
   in between here and the end of the function.  */
#define FP_RETURN(X)				\
do {						\
  register long r16 __asm__("16") = X##l;	\
  register long r17 __asm__("17") = X##h;	\
  asm volatile ("" : : "r"(r16), "r"(r17));	\
} while (0)
