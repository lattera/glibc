.file "roundf.s"

// Copyright (c) 2000, 2001, Intel Corporation
// All rights reserved.
// 
// Contributed 10/25/2000 by John Harrison, Cristina Iordache, Ted Kubaska,
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
// 10/25/2000: Created
//==============================================================
//
// API
//==============================================================
// float roundf(float x)
//

#include "libm_support.h"

// general input registers:  
//
roundf_GR_half      = r14
roundf_GR_big       = r15
roundf_GR_expmask   = r16
roundf_GR_signexp   = r17
roundf_GR_exp       = r18
roundf_GR_expdiff   = r19

// predicate registers used: 
// p6 - p10

// floating-point registers used: 

ROUNDF_NORM_f8        = f9                        
ROUNDF_TRUNC_f8       = f10
ROUNDF_RINT_f8        = f11
ROUNDF_FLOAT_TRUNC_f8 = f12
ROUNDF_FLOAT_RINT_f8  = f13
ROUNDF_REMAINDER      = f14
ROUNDF_HALF           = f15

// Overview of operation
//==============================================================

// float roundf(float x)
// Return an integer value (represented as a float) that is x 
// rounded to nearest integer, halfway cases rounded away from 
// zero. 
//  if x>0   result = trunc(x+0.5)
//  if x<0   result = trunc(x-0.5)
// *******************************************************************************

// Set denormal flag for denormal input and
// and take denormal fault if necessary.

// If x is NAN, ZERO, INFINITY, or >= 2^23 then return

// qnan snan inf norm     unorm 0 -+
// 1    1    1   0        0     1 11     0xe7


.align 32
.global roundf#

.section .text
.proc  roundf#
.align 32


roundf: 
	
// Get exponent for +0.5
// Truncate x to integer
{ .mfi
      addl           roundf_GR_half  = 0x0fffe, r0
      fcvt.fx.trunc.s1     ROUNDF_TRUNC_f8 = f8
      nop.i 999
}
	
// Get signexp of x
// Normalize input
// Form exponent mask
{ .mfi
      getf.exp  roundf_GR_signexp = f8
      fnorm     ROUNDF_NORM_f8 = f8                        
      addl      roundf_GR_expmask  = 0x1ffff, r0 ;;
}

// Form +0.5
// Round x to integer
{ .mfi
      setf.exp    ROUNDF_HALF  = roundf_GR_half                      
      fcvt.fx.s1  ROUNDF_RINT_f8 = f8
      nop.i 999 ;;
}
// Get exp of x
// Test for NAN, INF, ZERO
// Get exponent at which input has no fractional part
{ .mfi
      and         roundf_GR_exp = roundf_GR_expmask, roundf_GR_signexp
      fclass.m    p8,p9 = f8,0xe7
      addl        roundf_GR_big  = 0x10016, r0 ;;
}

// Get exp-bigexp
// If exp is so big there is no fractional part, then turn on p8, off p9
{ .mmi
      sub    roundf_GR_expdiff = roundf_GR_exp, roundf_GR_big ;;
#ifdef _LIBC
(p9)  cmp.lt.or.andcm  p8,p9 = r0, roundf_GR_expdiff
#else
(p9)  cmp.ge.or.andcm  p8,p9 = roundf_GR_expdiff, r0
#endif
      nop.i 999 ;;
}
     
// Set p6 if x<0, else set p7
{ .mfi
      nop.m 999
(p9)  fcmp.lt.unc  p6,p7 = f8,f0
      nop.i 999
}
	
// If NAN, INF, ZERO, or no fractional part, result is just normalized input
{ .mfi
      nop.m 999
(p8)  fnorm.s.s0  f8 = f8
      nop.i 999 ;;
}

// Float the truncated integer
{ .mfi
      nop.m 999
(p9)  fcvt.xf     ROUNDF_FLOAT_TRUNC_f8 = ROUNDF_TRUNC_f8
      nop.i 999 ;;
}

// Float the rounded integer to get preliminary result
{ .mfi
      nop.m 999
(p9)  fcvt.xf     ROUNDF_FLOAT_RINT_f8 = ROUNDF_RINT_f8
      nop.i 999 ;;
}

// If x<0 and the difference of the truncated input minus the input is 0.5
//    then result = truncated input - 1.0
// Else if x>0 and the difference of the input minus truncated input is 0.5
//    then result = truncated input + 1.0
// Else 
//    result = rounded input
// Endif
{ .mfi
      nop.m 999
(p6)  fsub.s1   ROUNDF_REMAINDER = ROUNDF_FLOAT_TRUNC_f8, ROUNDF_NORM_f8 
      nop.i 999
}
	
{ .mfi
      nop.m 999
(p7)  fsub.s1   ROUNDF_REMAINDER = ROUNDF_NORM_f8, ROUNDF_FLOAT_TRUNC_f8
      nop.i 999 ;;
}

// Assume preliminary result is rounded integer
{ .mfi
      nop.m 999
(p9)  fnorm.s.s0  f8 = ROUNDF_FLOAT_RINT_f8
      nop.i 999
}

// If x<0, test if result=0
{ .mfi
      nop.m 999
(p6)  fcmp.eq.unc  p10,p0 = ROUNDF_FLOAT_RINT_f8,f0
      nop.i 999 ;;
}

// If x<0 and result=0, set result=-0
{ .mfi
      nop.m 999
(p10) fmerge.ns  f8 = f1,f8
      nop.i 999
}
	
// If x<0, test if remainder=0.5
{ .mfi
      nop.m 999
(p6)  fcmp.eq.unc  p6,p0 = ROUNDF_REMAINDER, ROUNDF_HALF
      nop.i 999 ;;
}
	
// If x>0, test if remainder=0.5
{ .mfi
      nop.m 999
(p7)  fcmp.eq.unc  p7,p0 = ROUNDF_REMAINDER, ROUNDF_HALF
      nop.i 999 ;;
}

// If x<0 and remainder=0.5, result=truncated-1.0
// If x>0 and remainder=0.5, result=truncated+1.0
// Exit
.pred.rel "mutex",p6,p7
{ .mfi
      nop.m 999
(p6)  fsub.s.s0  f8 = ROUNDF_FLOAT_TRUNC_f8,f1
      nop.i 999 
}
	
{ .mfb
      nop.m 999
(p7)  fadd.s.s0  f8 = ROUNDF_FLOAT_TRUNC_f8,f1
      br.ret.sptk  b0 ;;
}

.endp roundf
ASM_SIZE_DIRECTIVE(roundf)
