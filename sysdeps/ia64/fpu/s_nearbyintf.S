.file "nearbyintf.s"

// Copyright (c) 2000, 2001, Intel Corporation
// All rights reserved.
// 
// Contributed 10/19/2000 by John Harrison, Cristina Iordache, Ted Kubaska,
// Bob Norin, Tom Rowan, Shane Story, and Ping Tak Peter Tang of the
// Computational Software Lab, Intel Corporation.
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
// 10/19/2000: Created
// 2/08/01  Corrected behavior for all rounding modes.
//==============================================================
//
// API
//==============================================================
// float nearbyintf(float x)

#include "libm_support.h"

//
// general registers used:  
//

nearbyint_GR_signexp   = r14
nearbyint_GR_exponent  = r15
nearbyint_GR_17ones    = r16
nearbyint_GR_10033     = r17
nearbyint_GR_fpsr      = r18
nearbyint_GR_rcs0      = r19
nearbyint_GR_rcs0_mask = r20


// predicate registers used: 
// p6-11

// floating-point registers used: 

NEARBYINT_NORM_f8      = f9                        
NEARBYINT_FLOAT_INT_f8 = f10
NEARBYINT_INT_f8       = f11

// Overview of operation
//==============================================================

// float nearbyintf(float x)
// Return an integer value (represented as a float) that is x rounded to integer in current
// rounding mode 
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
.global nearbyintf#

.section .text
.proc  nearbyintf#
.align 32


nearbyintf: 

{ .mfi
      mov nearbyint_GR_fpsr = ar40           // Read the fpsr--need to check rc.s0
      fcvt.fx.s1     NEARBYINT_INT_f8  = f8
      addl            nearbyint_GR_10033 = 0x10016, r0
}
{ .mfi
      nop.m 999
      fnorm.s1        NEARBYINT_NORM_f8  = f8
      mov         nearbyint_GR_17ones    = 0x1FFFF
;;
}

{ .mfi
      nop.m 999
      fclass.m.unc  p6,p0 = f8, 0xe7
      mov         nearbyint_GR_rcs0_mask  = 0x0c00
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
      fcvt.xf         NEARBYINT_FLOAT_INT_f8   = NEARBYINT_INT_f8
	nop.i 999
;;
}

{ .mfi
      getf.exp nearbyint_GR_signexp  = NEARBYINT_NORM_f8
      fcmp.eq.s0  p8,p0 = f8,f0      // Dummy op to set denormal
        nop.i 999
;;
}


{ .mii
	nop.m 999
	nop.i 999 
      and      nearbyint_GR_exponent = nearbyint_GR_signexp, nearbyint_GR_17ones
;;
}

{ .mmi
      cmp.ge.unc      p7,p6 = nearbyint_GR_exponent, nearbyint_GR_10033
      and nearbyint_GR_rcs0 = nearbyint_GR_rcs0_mask, nearbyint_GR_fpsr
	nop.i 999
;;
}

// Check to see if s0 rounding mode is round to nearest.  If not then set s2
// rounding mode to that of s0 and repeat conversions.
L(NEARBYINT_COMMON):
{ .mfb
      cmp.ne   p11,p0 = nearbyint_GR_rcs0, r0
(p6) fclass.m.unc   p9,p10  = NEARBYINT_FLOAT_INT_f8, 0x07  // Test for result=0
(p11) br.cond.spnt L(NEARBYINT_NOT_ROUND_NEAREST)  // Branch if not round to nearest
;;
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
(p9) fmerge.s f8 = f8, NEARBYINT_FLOAT_INT_f8
     nop.i 999
}
{ .mfb
      nop.m 999
(p10) fnorm.s f8 = NEARBYINT_FLOAT_INT_f8
     br.ret.sptk    b0
;;
}


L(NEARBYINT_NOT_ROUND_NEAREST):
// Set rounding mode of s2 to that of s0
{ .mfi
      mov nearbyint_GR_rcs0 = r0       // Clear so we don't come back here
      fsetc.s2     0x7f, 0x40
	nop.i 999
;;
}

{ .mfi
	nop.m 999
      fcvt.fx.s2     NEARBYINT_INT_f8  = f8
	nop.i 999
;;
}

{ .mfb
	nop.m 999
      fcvt.xf         NEARBYINT_FLOAT_INT_f8   = NEARBYINT_INT_f8
      br.cond.sptk  L(NEARBYINT_COMMON)
;;
}


.endp nearbyintf
ASM_SIZE_DIRECTIVE(nearbyintf)
