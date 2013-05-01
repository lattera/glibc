/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * finite(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include <math.h>
#include <math_private.h>
#include <stdint.h>

#undef __finite
int
__finite(double x)
{
  int64_t lx;
  EXTRACT_WORDS64(lx,x);
  return (int)((uint64_t)((lx&INT64_C(0x7fffffffffffffff))-INT64_C(0x7ff0000000000000))>>63);
}
hidden_def (__finite)
weak_alias (__finite, finite)
#ifdef NO_LONG_DOUBLE
strong_alias (__finite, __finitel)
weak_alias (__finite, finitel)
#endif
