.file "rintf.s"

// Copyright (c) 2000, 2001, Intel Corporation
// All rights reserved.
// 
// Contributed 2/2/2000 by John Harrison, Ted Kubaska, Bob Norin, Shane Story,
// and Ping Tak Peter Tang of the Computational Software Lab, Intel Corporation.
// 
// WARRANTY DISCLAIMER
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
// 
// Intel Corporation is the author of this code, and requests that all
// problem reports or change requests be submitted to it directly at 
// http://developer.intel.com/opensource.
//
// History
//==============================================================
// 2/02/00: Initial version
// 2/08/01  Corrected behavior for all rounding modes.
//
// API
//==============================================================
// float rintf(float x)

#include "libm_support.h"

//
// general registers used:  
//
rint_GR_FFFF      = r14
rint_GR_signexp   = r15
rint_GR_exponent  = r16
rint_GR_17ones    = r17
rint_GR_10033     = r18
rint_GR_fpsr      = r19
rint_GR_rcs0      = r20
rint_GR_rcs0_mask = r21


// predicate registers used: 
// p6-11

// floating-point registers used: 

RINT_NORM_f8      = f9                        
RINT_FFFF         = f10 
RINT_INEXACT      = f11 
RINT_FLOAT_INT_f8 = f12
RINT_INT_f8       = f13

// Overview of operation
//==============================================================

// float rintf(float x)
// Return an integer value (represented as a float) that is x rounded to integer in current
// rounding mode 
// Inexact is set if x != rintf(x)
// *******************************************************************************

// Set denormal flag for denormal input and
// and take denormal fault if necessary.

// Is the input an integer value already?

// double_extended
// if the exponent is >= 1003e => 3F(true) = 63(decimal)
// we have a significand of 64 bits 1.63-bits.
// If we multiply by 2^63, we no longer have a fractional part
// So input is an integer value already.

// double
// if the exponent is >= 10033 => 34(true) = 52(decimal)
// 34 + 3ff = 433
// we have a significand of 53 bits 1.52-bits. (implicit 1)
// If we multiply by 2^52, we no longer have a fractional part
// So input is an integer value already.

// single
// if the exponent is >= 10016 => 17(true) = 23(decimal)
// we have a significand of 53 bits 1.52-bits. (implicit 1)
// If we multiply by 2^52, we no longer have a fractional part
// So input is an integer value already.

// If x is NAN, ZERO, or INFINITY, then  return

// qnan snan inf norm     unorm 0 -+
// 1    1    1   0        0     1 11     0xe7


.align 32
.global rintf#

.section .text
.proc  rintf#
.align 32


rintf:
#ifdef _LIBC
.global __rintf
.type __rintf,@function
__rintf:
#endif

{ .mfi
      mov rint_GR_fpsr = ar40           // Read the fpsr--need to check rc.s0
      fcvt.fx.s1     RINT_INT_f8  = f8
      addl            rint_GR_10033 = 0x10016, r0
}
{ .mfi
      mov        rint_GR_FFFF      = -1
      fnorm.s1        RINT_NORM_f8  = f8
      mov         rint_GR_17ones    = 0x1FFFF
;;
}

{ .mfi
      setf.sig    RINT_FFFF  = rint_GR_FFFF
      fclass.m.unc  p6,p0 = f8, 0xe7
      mov         rint_GR_rcs0_mask  = 0x0c00
;;
}

{ .mfb
	nop.m 999
(p6)  fnorm.s f8 = f8
(p6)  br.ret.spnt   b0    // Exit if x nan, inf, zero
;;
}

{ .mfi
	nop.m 999
      fcvt.xf         RINT_FLOAT_INT_f8   = RINT_INT_f8
	nop.i 999
;;
}

{ .mfi
      getf.exp rint_GR_signexp  = RINT_NORM_f8
      fcmp.eq.s0  p8,p0 = f8,f0      // Dummy op to set denormal
        nop.i 999
;;
}


{ .mii
	nop.m 999
	nop.i 999 
      and      rint_GR_exponent = rint_GR_signexp, rint_GR_17ones
;;
}

{ .mmi
      cmp.ge.unc      p7,p6 = rint_GR_exponent, rint_GR_10033
      and rint_GR_rcs0 = rint_GR_rcs0_mask, rint_GR_fpsr
	nop.i 999
;;
}

// Check to see if s0 rounding mode is round to nearest.  If not then set s2
// rounding mode to that of s0 and repeat conversions.
L(RINT_COMMON):
{ .mfb
      cmp.ne   p11,p0 = rint_GR_rcs0, r0
(p6) fclass.m.unc   p9,p10  = RINT_FLOAT_INT_f8, 0x07  // Test for result=0
(p11) br.cond.spnt L(RINT_NOT_ROUND_NEAREST)  // Branch if not round to nearest
;;
}

{ .mfi
	nop.m 999
(p6) fcmp.eq.unc.s1  p0,p8  = RINT_FLOAT_INT_f8, RINT_NORM_f8
	nop.i 999
}
{ .mfi
	nop.m 999
(p7) fnorm.s.s0   f8 = f8
	nop.i 999
;;
}

// If result is zero, merge sign of input
{ .mfi
     nop.m 999
(p9) fmerge.s f8 = f8, RINT_FLOAT_INT_f8
     nop.i 999
}
{ .mfi
      nop.m 999
(p10) fnorm.s f8 = RINT_FLOAT_INT_f8
     nop.i 999
;;
}

{ .mfb
     nop.m 999
(p8) fmpy.s0     RINT_INEXACT = RINT_FFFF,RINT_FFFF  // Dummy to set inexact
     br.ret.sptk    b0
;;
}

L(RINT_NOT_ROUND_NEAREST):
// Set rounding mode of s2 to that of s0
{ .mfi
      mov rint_GR_rcs0 = r0       // Clear so we don't come back here
      fsetc.s2     0x7f, 0x40
	nop.i 999
;;
}

{ .mfi
	nop.m 999
      fcvt.fx.s2     RINT_INT_f8  = f8
	nop.i 999
;;
}

{ .mfb
	nop.m 999
      fcvt.xf         RINT_FLOAT_INT_f8   = RINT_INT_f8
      br.cond.sptk  L(RINT_COMMON)
;;
}


.endp rintf
ASM_SIZE_DIRECTIVE(rintf)
#ifdef _LIBC
ASM_SIZE_DIRECTIVE(__rintf)
#endif
