.file "ceill.s"

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

#include "libm_support.h"

.align 32
.global ceill#

.section .text
.proc  ceill#
.align 32

// History
//==============================================================
// 2/02/00: Initial version
// 6/13/00: Improved speed
// 6/27/00: Eliminated incorrect invalid flag setting

// API
//==============================================================
// double ceill(double x)

// general input registers:  

ceil_GR_FFFF      = r14
ceil_GR_signexp   = r15
ceil_GR_exponent  = r16
ceil_GR_expmask   = r17
ceil_GR_bigexp    = r18


// predicate registers used: 

// p6  ==> Input is NaN, infinity, zero
// p7  ==> Input is denormal
// p8  ==> Input is <0
// p9  ==> Input is >=0
// p10 ==> Input is already an integer (bigger than largest integer)
// p11 ==> Input is not a large integer
// p12 ==> Input is a smaller integer
// p13 ==> Input is not an even integer, so inexact must be set
// p14 ==> Input is between -1 and 0, so result will be -0 and inexact


// floating-point registers used: 

CEIL_SIGNED_ZERO  = f7
CEIL_NORM_f8      = f9                        
CEIL_FFFF         = f10 
CEIL_INEXACT      = f11 
CEIL_FLOAT_INT_f8 = f12
CEIL_INT_f8       = f13
CEIL_adj          = f14
CEIL_MINUS_ONE    = f15

// Overview of operation
//==============================================================

// long double ceill(long double x)
// Return an integer value (represented as a long double) that is the smallest 
// value not less than x
// This is x rounded toward +infinity to an integral value.
// Inexact is set if x != ceill(x)
// **************************************************************************

// Set denormal flag for denormal input and
// and take denormal fault if necessary.

// Is the input an integer value already?

// double_extended
// if the exponent is > 1003e => 3F(true) = 63(decimal)
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
// if the exponent is > 10016 => 17(true) = 23(decimal)
// we have a significand of 24 bits 1.23-bits. (implicit 1)
// If we multiply by 2^23, we no longer have a fractional part
// So input is an integer value already.

// If x is NAN, ZERO, or INFINITY, then  return

// qnan snan inf norm     unorm 0 -+
// 1    1    1   0        0     1 11     0xe7


ceill:

{ .mfi
      getf.exp ceil_GR_signexp  = f8
      fcvt.fx.trunc.s1     CEIL_INT_f8  = f8
      addl        ceil_GR_bigexp = 0x1003e, r0
}
{ .mfi
      addl        ceil_GR_FFFF      = -1,r0
      fcmp.lt.s1  p8,p9 = f8,f0
      mov         ceil_GR_expmask    = 0x1FFFF ;;
}

// p7 ==> denorm
{ .mfi
      setf.sig    CEIL_FFFF  = ceil_GR_FFFF
      fclass.m    p7,p0 = f8, 0x0b
      nop.i 999
}
{ .mfi
      nop.m 999
      fnorm           CEIL_NORM_f8  = f8
      nop.i 999 ;;
}

// Form 0 with sign of input in case negative zero is needed
{ .mfi
      nop.m 999
      fmerge.s           CEIL_SIGNED_ZERO = f8, f0
      nop.i 999
}
{ .mfi
      nop.m 999
      fsub.s1           CEIL_MINUS_ONE = f0, f1
      nop.i 999 ;;
}

// p6 ==> NAN, INF, ZERO
{ .mfb
      nop.m 999
      fclass.m      p6,p10 = f8, 0xe7
(p7)  br.cond.spnt  L(CEIL_DENORM) ;;
}

L(CEIL_COMMON):
.pred.rel "mutex",p8,p9
// Set adjustment to add to trunc(x) for result
//   If x>0,  adjustment is 1.0
//   If x<=0, adjustment is 0.0
{ .mfi
      and      ceil_GR_exponent = ceil_GR_signexp, ceil_GR_expmask
(p9)  fadd.s1  CEIL_adj = f1,f0
      nop.i 999
}
{ .mfi
      nop.m 999
(p8)  fadd.s1  CEIL_adj = f0,f0
      nop.i 999 ;;
}

{ .mfi
(p10) cmp.ge.unc    p10,p11 = ceil_GR_exponent, ceil_GR_bigexp
(p6)  fnorm   f8 = f8
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p11) fcvt.xf         CEIL_FLOAT_INT_f8   = CEIL_INT_f8
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10) fnorm   f8 = CEIL_NORM_f8
      nop.i 999 ;;
}

// Is -1 < x < 0?  If so, result will be -0.  Special case it with p14 set.
{ .mfi
      nop.m 999
(p8)  fcmp.gt.unc.s1 p14,p0 = CEIL_NORM_f8, CEIL_MINUS_ONE
      nop.i 999 ;;
}

{ .mfi
(p14) cmp.ne  p11,p0 = r0,r0
(p14) fnorm   f8 = CEIL_SIGNED_ZERO
      nop.i 999
}
{ .mfi
      nop.m 999
(p14) fmpy.s0     CEIL_INEXACT = CEIL_FFFF,CEIL_FFFF
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p11) fadd     f8 = CEIL_FLOAT_INT_f8,CEIL_adj
      nop.i 999 ;;
}
{ .mfi
      nop.m 999
(p11) fcmp.eq.unc.s1  p12,p13  = CEIL_FLOAT_INT_f8, CEIL_NORM_f8
      nop.i 999 ;;
}

// Set inexact if result not equal to input
{ .mfi
      nop.m 999
(p13) fmpy.s0     CEIL_INEXACT = CEIL_FFFF,CEIL_FFFF
      nop.i 999
}
// Set result to input if integer
{ .mfb
      nop.m 999
(p12) fnorm   f8 = CEIL_NORM_f8
      br.ret.sptk    b0 ;;
}

// Here if input denorm
L(CEIL_DENORM):
{ .mfb
      getf.exp ceil_GR_signexp  = CEIL_NORM_f8
      fcvt.fx.trunc.s1     CEIL_INT_f8  = CEIL_NORM_f8
      br.cond.sptk  L(CEIL_COMMON) ;;
}

.endp ceill
ASM_SIZE_DIRECTIVE(ceill)
