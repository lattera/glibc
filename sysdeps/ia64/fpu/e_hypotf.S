.file "hypotf.asm"

// Copyright (c) 2000, 2001, Intel Corporation
// All rights reserved.
// 
// Contributed 2/2/2000 by John Harrison, Cristina Iordache, Ted Kubaska, 
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
// *********************************************************************
//
// History: 
// 2/02/00  hand-optimized
// 4/04/00  Unwind support added
// 6/26/00  new version
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
//
// *********************************************************************
//                           ___________
// Function:   hypotf(x,y) = |(x^2 + y^2) = for single precision values
//             x and y
//             Also provides cabsf functionality.
//
// *********************************************************************
//
// Resources Used:
//
//    Floating-Point Registers: f8  (Input and Return Value)
//                              f9  (Input)
//                              f6 -f15
//
//    General Purpose Registers:
//      r2-r3   (Scratch)
//      r32-r36 (Locals)
//      r37-r40 (Used to pass arguments to error handling routine)
//
//    Predicate Registers:      p6 - p10
//
// *********************************************************************
//
// IEEE Special Conditions:
//
//    All faults and exceptions should be raised correctly.
//    Overflow can occur.
//    hypotf(Infinity and anything) = +Infinity
//    hypotf(QNaN and anything) = QNaN
//    hypotf(SNaN and anything ) = QNaN
//
// *********************************************************************
//
// Implementation:
//  x2 = x * x   in double-extended
//  y2 = y * y   in double-extended
//  temp = x2 + y2   in double-extended
//  sqrt(temp) rounded to single precision 
//
// *********************************************************************

#include "libm_support.h"

GR_SAVE_PFS         = r33
GR_SAVE_B0          = r34
GR_SAVE_GP          = r35
GR_Parameter_X      = r36
GR_Parameter_Y      = r37
GR_Parameter_RESULT = r38
GR_Parameter_TAG    = r39

FR_X                = f14
FR_Y                = f15
FR_RESULT           = f8

.section .text
#ifndef _LIBC
.proc cabsf#
.global cabsf#
cabsf: 
.endp cabsf
#endif
.proc hypotf#
.global hypotf#
.align 64 

hypotf: 
#ifdef _LIBC
.global __hypotf
__hypotf:
.global __ieee754_hypotf
__ieee754_hypotf:
#endif 
{.mfi
  alloc r32= ar.pfs,0,4,4,0
  // Compute x*x
  fma.s1 f10=f8,f8,f0
  // r2=bias-1
  mov r2=0xfffe 
}
{.mfi
  nop.m 0
  // y*y
  fma.s1 f11=f9,f9,f0
  nop.i 0;;
}

{ .mfi
     nop.m 0
//   Check if x is an Inf - if so return Inf even
//   if y is a NaN (C9X)
     fclass.m.unc p7, p6 = f8, 0x023
     nop.i 0
}
{.mfi
  nop.m 0
  // if possible overflow, copy f8 to f14
  // set Denormal, if necessary 
  // (p8) 
  fma.s.s0 f14=f8,f1,f0
  nop.i 0;;
}

{ .mfi
     nop.m 0
//   Check if y is an Inf - if so return Inf even
//   if x is a NaN (C9X)
     fclass.m.unc p8, p9 = f9, 0x023
	 nop.i 0
}
{ .mfi
     nop.m 0
//   For x=inf, multiply y by 1 to raise invalid on y an SNaN
//   (p7) fma.s0 f9=f9,f1,f0
     // copy f9 to f15; set Denormal, if necessary
	 fma.s.s0 f15=f9,f1,f0
     nop.i 0;;
}
{.mfi
  nop.m 0
  // is y Zero ?
  (p6) fclass.m p6,p0=f9,0x7
  nop.i 0;;
}
{.mfi
  nop.m 0
  // is x Zero ?
  (p9) fclass.m p9,p0=f8,0x7
  nop.i 0;;
}

{.mfi
  // f7=0.5
  setf.exp f7=r2
  // a=x2+y2
  fma.s1 f12=f10,f1,f11
  nop.i 0;;
}

{.mfi
  nop.m 0
  // x not NaN ?
  (p6) fclass.m p7,p0=f8,0x3f
  nop.i 0
}
{.mfi
  // 2*emax-2
  mov r2=0x100fb
  // f6=2
  fma.s1 f6=f1,f1,f1
  nop.i 0;;
}

{.mfi
  nop.m 0
  // y not NaN ?
  (p9) fclass.m p8,p0=f9,0x3f
  nop.i 0;;
}

{.mfb
  nop.m 0
  // if f8=Infinity or f9=Zero, return |f8|
  (p7) fmerge.s f8=f0,f14
  (p7) br.ret.spnt b0
}
{.mfb
  nop.m 0
  // if f9=Infinity or f8=Zero, return |f9|
  (p8) fmerge.s f8=f0,f15
  (p8) br.ret.spnt b0;;
}

{ .mfi
	 nop.m 0
//   Identify Natvals, Infs, NaNs, and Zeros 
//   and return result
     fclass.m.unc p7, p0 = f12, 0x1E7
     nop.i 0
} 
{.mfi
  nop.m 0
  // z0=frsqrta(a)
  frsqrta.s1 f8,p6=f12
  nop.i 0;;
}

{.mfb
  // get exponent of x^2+y^2
  getf.exp r3=f12
  // if special case, set f8
  (p7) mov f8=f12
  (p7) br.ret.spnt b0;;
}


{.mfi
  nop.m 0
  // S0=a*z0
  (p6) fma.s1 f12=f12,f8,f0
  nop.i 0
}
{.mfi
  nop.m 0
  // H0=0.5*z0
  (p6) fma.s1 f10=f8,f7,f0
  nop.i 0;;
} 


{.mfi
  nop.m 0
  // f6=5/2
  fma.s1 f6=f7,f1,f6
  nop.i 0
}
{.mfi
  nop.m 0
  // f11=3/2
  fma.s1 f11=f7,f1,f1
  nop.i 0;;
}

{.mfi
  nop.m 0
  // d=0.5-S0*H0
  (p6) fnma.s1 f7=f12,f10,f7
  nop.i 0;;
}

{.mfi
  nop.m 0
  // P01=d+1
  (p6) fma.s1 f10=f1,f7,f1
  nop.i 0
}
{.mfi
  nop.m 0
  // P23=5/2*d+3/2
  (p6) fma.s1 f11=f6,f7,f11
  nop.i 0;;
}
{.mfi
  nop.m 0
  // d2=d*d
  (p6) fma.s1 f7=f7,f7,f0
  nop.i 0;;
}


{.mfi
  // Is x^2 + y^2 well less than the overflow 
  // threshold?
  (p6) cmp.lt.unc p7, p8 =  r3,r2
  // P=P01+d2*P23
  (p6) fma.s1 f10=f7,f11,f10
  nop.i 0;;
}

{.mfb
  nop.m 0
  // S=P*S0
  fma.s.s0 f8=f10,f12,f0
  // No overflow in this case
  (p7) br.ret.sptk b0;;
}

{ .mfi
     nop.m 0  
(p8) fsetc.s2 0x7F,0x42 
     // Possible overflow path, must detect by
     // Setting widest range exponent with prevailing
     // rounding mode.
     nop.i 0 ;;
}


{ .mfi
   // bias+0x400 (bias+EMAX+1)
   (p8) mov r2=0x1007f
   // S=P*S0
   (p8) fma.s.s2 f12=f10,f12,f0
   nop.i 0 ;;
}
{ .mfi
(p8) setf.exp f11 = r2
(p8) fsetc.s2 0x7F,0x40
//   Restore Original Mode in S2
     nop.i 0 ;;
}
{ .mfi
     nop.m 0 
(p8) fcmp.lt.unc.s1 p9, p10 =  f12, f11
     nop.i 0 ;;
}
{ .mib
     nop.m 0
     mov GR_Parameter_TAG = 47
	 // No overflow
(p9) br.ret.sptk b0;; 
}
.endp hypotf
ASM_SIZE_DIRECTIVE(hypotf)

.proc __libm_error_region
__libm_error_region:
.prologue
{ .mii
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
(p0)    mov   GR_Parameter_TAG = 47                   
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs                 // Save ar.pfs
}
{ .mfi
.fframe 64
        add sp=-64,sp                           // Create new stack
        nop.f 0
        mov GR_SAVE_GP=gp                       // Save gp
};;
{ .mmi
        stfs [GR_Parameter_Y] = FR_Y,16         // Store Parameter 2 on stack
        add GR_Parameter_X = 16,sp              // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                       // Save b0
};;
.body
{ .mib
        stfs [GR_Parameter_X] = FR_X            // Store Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y
        nop.b 0                                 // Parameter 3 address
}
{ .mib
        stfs [GR_Parameter_Y] = FR_RESULT       // Store Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#   // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;
{ .mmi
        ldfs  f8 = [GR_Parameter_RESULT]       // Get return result off stack
.restore sp
        add   sp = 64,sp                       // Restore stack pointer
        mov   b0 = GR_SAVE_B0                  // Restore return address
};;
{ .mib
        mov   gp = GR_SAVE_GP                  // Restore gp 
        mov   ar.pfs = GR_SAVE_PFS             // Restore ar.pfs
        br.ret.sptk     b0                     // Return
};; 

.endp __libm_error_region
ASM_SIZE_DIRECTIVE(__libm_error_region)

.type   __libm_error_support#,@function
.global __libm_error_support#
