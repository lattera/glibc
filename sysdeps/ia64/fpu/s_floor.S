.file "floor.s"

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
.align 32
.global floor#

.section .text
.proc  floor#
.align 32

// History
//==============================================================
// 2/02/00: Initial version
// 3/22/00: Updated to improve performance 
// 6/13/00: Improved speed, fixed setting of inexact flag
// 6/27/00: Eliminated incorrect invalid flag setting
// 2/07/01: Corrected sign of zero result in round to -inf mode

// API
//==============================================================
// double floor(double x)

// general input registers:  

floor_GR_FFFF      = r14
floor_GR_signexp   = r15
floor_GR_exponent  = r16
floor_GR_expmask   = r17
floor_GR_bigexp    = r18


// predicate registers used: 

// p6  ==> Input is NaN, infinity, zero
// p7  ==> Input is denormal
// p8  ==> Input is <0
// p9  ==> Input is >=0
// p10 ==> Input is already an integer (bigger than largest integer)
// p11 ==> Input is not a large integer
// p12 ==> Input is a smaller integer
// p13 ==> Input is not an even integer, so inexact must be set


// floating-point registers used: 

FLOOR_NORM_f8      = f9                        
FLOOR_FFFF         = f10 
FLOOR_INEXACT      = f11 
FLOOR_FLOAT_INT_f8 = f12
FLOOR_INT_f8       = f13
FLOOR_adj          = f14

// Overview of operation
//==============================================================

// double floor(double x)
// Return an integer value (represented as a double) that is the largest 
// value not greater than x
// This is x rounded toward -infinity to an integral value.
// Inexact is set if x != floor(x)
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

#include "libm_support.h"

floor:
#ifdef _LIBC
.global __floor
__floor:
#endif

{ .mfi
      getf.exp floor_GR_signexp  = f8
      fcvt.fx.trunc.s1     FLOOR_INT_f8  = f8
      addl        floor_GR_bigexp = 0x10033, r0
}
{ .mfi
      addl        floor_GR_FFFF      = -1,r0
      fcmp.lt.s1  p8,p9 = f8,f0
      mov         floor_GR_expmask    = 0x1FFFF ;;
}

// p7 ==> denorm
{ .mfi
      setf.sig    FLOOR_FFFF  = floor_GR_FFFF
      fclass.m    p7,p0 = f8, 0x0b
      nop.i 999
}
{ .mfi
      nop.m 999
      fnorm.s1           FLOOR_NORM_f8  = f8
      nop.i 999 ;;
}

// p6 ==> NAN, INF, ZERO
{ .mfb
      nop.m 999
      fclass.m      p6,p10 = f8, 0xe7
(p7)  br.cond.spnt  L(FLOOR_DENORM) ;;
}

L(FLOOR_COMMON):
.pred.rel "mutex",p8,p9
// Set adjustment to subtract from trunc(x) for result
//   If x<0,  adjustment is -1.0
//   If x>=0, adjustment is 0.0
{ .mfi
      and      floor_GR_exponent = floor_GR_signexp, floor_GR_expmask
(p8)  fnma.s1  FLOOR_adj = f1,f1,f0
      nop.i 999
}
{ .mfi
      nop.m 999
(p9)  fadd.s1  FLOOR_adj = f0,f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
      fcmp.eq.s0  p12,p0 = f8,f0 // Dummy op to set denormal and invalid flag
      nop.i 999
}
{ .mfi
(p10) cmp.ge.unc    p10,p11 = floor_GR_exponent, floor_GR_bigexp
(p6)  fnorm.d f8 = f8
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p11) fcvt.xf         FLOOR_FLOAT_INT_f8   = FLOOR_INT_f8
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10) fnorm.d f8 = FLOOR_NORM_f8
      nop.i 999 ;;
}


{ .mfi
      nop.m 999
(p11) fadd.d   f8 = FLOOR_FLOAT_INT_f8,FLOOR_adj
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p11) fcmp.eq.unc.s1  p12,p13  = FLOOR_FLOAT_INT_f8, FLOOR_NORM_f8
      nop.i 999 ;;
}

// Set inexact if result not equal to input
{ .mfi
      nop.m 999
(p13) fmpy.s0     FLOOR_INEXACT = FLOOR_FFFF,FLOOR_FFFF
      nop.i 999
}
// Set result to input if integer
{ .mfb
      nop.m 999
(p12) fnorm.d f8 = FLOOR_NORM_f8
      br.ret.sptk    b0 ;;
}

// Here if input denorm
L(FLOOR_DENORM):
{ .mfb
      getf.exp floor_GR_signexp  = FLOOR_NORM_f8
      fcvt.fx.trunc.s1     FLOOR_INT_f8  = FLOOR_NORM_f8
      br.cond.sptk  L(FLOOR_COMMON) ;;
}

.endp floor
ASM_SIZE_DIRECTIVE(floor)
