/* For each N divided by D, we do:
      result = (double) N / (double) D
   Then, for each N mod D, we do:
      result = N - (D * divMODE (N, D))

   FIXME:
   The q and qu versions won't deal with operands > 50 bits.  We also
   don't check for divide by zero.  */

#include "DEFS.h"
#if 0
/* We do not handle div by zero yet.  */
#include <machine/pal.h>
#endif
#include <regdef.h>

define(path, `SYSDEP_DIR/macros.m4')dnl
include(path)

FUNC__(OP)
	! First set up the dividend.
	EXTEND(t10)
	stq t10,0(sp)
	ldt $f10,0(sp)
	cvtqt $f10,$f10
	ADJQU($f10)

	! Then set up the divisor.
	EXTEND(t11)
	stq t11,0(sp)
	ldt $f1,0(sp)
	cvtqt $f1,$f1
	ADJQU($f1)

	! Do the division.
	divt $f10,$f1,$f10
	cvttqc $f10,$f10

	! Put the result in t12.
	stt $f10,0(sp)
	ldq t12,0(sp)
	FULLEXTEND(t12)

	DOREM

	lda sp,16(sp)
	ret zero,(t9),1
	.end NAME__(OP)
