.file "truncf.s"

// Copyright (c) 2000, 2001, Intel Corporation
// All rights reserved.
// 
// Contributed 7/7/2000 by John Harrison, Cristina Iordache, Ted Kubaska,
// Bob Norin, Shane Story, and Ping Tak Peter Tang of the
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
.align 32
.global truncf#

.section .text
.proc  truncf#
.align 32

// History
//==============================================================
// 7/7/00: Created 
//==============================================================

// API
//==============================================================
// float truncf(float x)
//==============================================================

#include "libm_support.h"

// general input registers:  
TRUNC_GR_FFFF      = r14
TRUNC_GR_signexp   = r15
TRUNC_GR_exponent  = r16
TRUNC_GR_expmask   = r17
TRUNC_GR_bigexp    = r18

// floating-point registers:
// f8, f9, f11, f12

// predicate registers used: 
// p6, p7, p8, p9, p10, p11

// Overview of operation
//==============================================================
// float truncf(float x)
// Return an integer value (represented as a float) less than or 
// equal to x in magnitude.
// This is x rounded toward zero to an integral value.
//==============================================================

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

truncf:

{ .mfi
      getf.exp         TRUNC_GR_signexp  = f8
      fcvt.fx.trunc.s1 f9  = f8
      addl             TRUNC_GR_bigexp = 0x10016, r0
}
{ .mfi
      mov              TRUNC_GR_FFFF    = 0x0FFFF
      fnorm.s          f11  = f8
      mov              TRUNC_GR_expmask    = 0x1FFFF 
};;
//    get the exponent of x
//    convert x to integer in signficand of f9 
//    Normalize x - this will raise invalid on SNaNs, the
//    denormal operand flag - and possibly a spurious U flag
//    get exponent only mask (will exclude sign bit)

{ .mfi
      nop.m            0
      fclass.m         p7,p8 = f8, 0x0b  
      nop.i            0
}
{ .mfi
      nop.m            0
      fcmp.eq.unc.s1   p9,p0 = f8,f0
      nop.i      0       
};;
//    fclass to set p7 if unnorm 
{ .mmi
      and              TRUNC_GR_exponent = TRUNC_GR_signexp, TRUNC_GR_expmask ;;
(p8)  cmp.ge.unc       p10,p11 = TRUNC_GR_exponent, TRUNC_GR_bigexp
(p8)  cmp.ne.unc       p6,p0 = TRUNC_GR_exponent, TRUNC_GR_signexp
};;
//    Get the exponent of x
//    Test if exponent such that result already an integer
//    Test if x < 0
{ .mmi
(p9)  cmp.eq.andcm     p10,p11 = r0, r0 
(p6)  cmp.lt.unc       p6,p0 = TRUNC_GR_exponent, TRUNC_GR_FFFF
      nop.i 0 
};;
//    If -1 < x < 0, set p6, turn off p10 and p11, and set result to -0.0
{ .mfb
(p6)  cmp.eq.andcm     p10,p11 = r0, r0 
(p6)  fmerge.s         f8 = f8, f0
      nop.b 0
};;
//    If not a unnorm, set p10 if x already is a big int, nan, or inf?
//    If not a unnorm, set p10 if x already is a big int, nan, or inf?
.pred.rel "mutex",p10,p11    
{ .mfb
      nop.m            0
(p11) fcvt.xf          f8   = f9 
      nop.b            0 
}
{ .mfb
      nop.m            0
(p10) fma.s.s1         f8   = f11,f1,f0 
(p8)  br.ret.sptk      b0 
};;
//    If not a unnorm and not an big int, nan,or +/-inf convert signficand
//    back to f8.
//    If not a unorm  and a big int, nan, or +/-inf, return fnorm'd x 
//    If not a unorm, Return
//    If unnorm, get the exponent again - perhaps it wasn't a denorm. 
{ .mfb
(p7)  getf.exp         TRUNC_GR_signexp  = f11
(p7)  fcvt.fx.trunc.s1 f12   = f11 
      nop.b 0
};;
{ .mfb
      and              TRUNC_GR_exponent = TRUNC_GR_signexp, TRUNC_GR_expmask
      fcmp.lt.unc.s1   p9,p0 = f8,f0
      nop.b            0
};;
{ .mfb
      cmp.ge.unc       p10,p11 = TRUNC_GR_exponent, TRUNC_GR_bigexp
      nop.f            0
      nop.b            0
};;
// If a unnorm, check to see if value is already a big int. 
{ .mfb
      nop.m 0
(p11) fcvt.xf          f8   = f12 
      nop.b 0
}
{ .mfi
      nop.m 0
(p10) fma.s.s1         f8   = f11,f1,f0 
      nop.i 0
};;
{ .mfb
      nop.m 0
(p9)  fmerge.ns        f8   = f1,f8 
      br.ret.sptk      b0 
};;
// If so return it. Otherwise, return (fcvt.xf(fcvt.fx.trunc(x)))
// Make sure the result is negative if it should be - that is
// negative(denormal) -> -0.
.endp truncf
ASM_SIZE_DIRECTIVE(truncf)
