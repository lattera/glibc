.file "logb.s"

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
// 2/02/00  Initial version
// 2/16/00  Modified to conform to C9X 
// 3/16/00  Improved speed
// 4/04/00  Unwind support added
// 5/30/00  Fixed bug when x double-extended denormal
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
//
// API
//==============================================================
// double logb( double x);
//
// Overview of operation
//==============================================================
// The logb function extracts the exponent of x as an integer in 
// floating-point format. 
// logb computes log2 of x as a double
//
// logb is similar to ilogb but differs in the  following ways:
//         +-inf
//            ilogb: returns INT_MAX
//             logb: returns +inf
//         Nan  returns FP_LOGBNAN (which is either INT_MAX or INT_MIN)
//            ilogb: returns INT_MAX (7fffffff)
//             logb: returns QNAN (quietized SNAN)
//         0    returns FP_ILOGB0 (which is either INT_MIN or -INT_MAX)
//            ilogb: returns -INT_MAX (80000001)
//             logb: returns -inf, raises the divide-by-zero exception,
//                   and calls libm_error_support to set domain error
//
// Registers used
//==============================================================
// general registers used: 
// ar.pfs r32 
// r33 -> r37
// r38 -> r41 used as parameters to error path
//
// predicate registers used: 
// p6, p7, p8
// floating-point registers used: 
// f9, f10, f11
// f8, input

#include "libm_support.h"

GR_SAVE_B0          = r34
GR_SAVE_GP          = r35
GR_SAVE_PFS         = r32

GR_Parameter_X      = r38
GR_Parameter_Y      = r39
GR_Parameter_RESULT = r40

.align 32
.global logb#

.section .text
.proc  logb#
.align 32


logb: 

// qnan snan inf norm     unorm 0 -+
// 0    0    0   0        1     0 11
// 0                      b
{ .mfi
      alloc          r32=ar.pfs,1,5,4,0
(p0)  fclass.m.unc  p8,p0 = f8, 0x0b
      nop.i 999
}
// X NORMAL
// r37 = exp(f8) - - 0xffff
// sig(f8) = r37
// f8 = convert_to_fp (sig))
{ .mfi
(p0)  getf.exp      r35 = f8
(p0)  fnorm         f10=f8
      nop.i 999 ;;
}

// qnan snan inf norm     unorm 0 -+
// 1    1    1   0        0     0 11
// e                      3
{ .mmf
(p0)  mov      r33 = 0xffff
(p0)  mov      r34 = 0x1ffff
(p0)  fclass.m.unc  p6,p0 = f8, 0xe3 ;;
}

{ .mfb
(p0)  and           r36 = r35, r34
(p0)  fclass.m.unc  p7,p0 = f8, 0x07
(p8)  br.cond.spnt  L(LOGB_DENORM) ;;
}

{ .mib
(p0)  sub           r37 = r36, r33
      nop.i 999
(p6)  br.cond.spnt  L(LOGB_NAN_INF) ;;
}

{ .mib
(p0)  setf.sig      f9  = r37
      nop.i 999
(p7)  br.cond.spnt  L(LOGB_ZERO) ;;
}

{ .mfi
      nop.m 999
(p0)  fcvt.xf       f10 = f9
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p0)  fnorm.d       f8 = f10
(p0)  br.ret.sptk    b0 ;;
}

L(LOGB_DENORM):
// Form signexp of 2^64 in case need to scale denormal
// Check to see if double-extended denormal
{ .mfi
(p0)  mov r38 = 0x1003f
(p0)  fclass.m.unc  p8,p0 = f10, 0x0b
      nop.i 999 ;;
}

// Form 2^64 in case need to scale denormal
{ .mfi
(p0)  setf.exp f11 = r38
      nop.f 999
      nop.i 999 ;;
}

// If double-extended denormal add 64 to exponent bias for scaling
// If double-extended denormal form x * 2^64 which is normal
{ .mfi
(p8)  add r33 = 64, r33
(p8)  fmpy f10 = f10, f11    
      nop.i 999 ;;
}

// Logic is the same as normal path but use normalized input
{ .mmi
(p0)  getf.exp      r35 = f10 ;;
      nop.m 999
      nop.i 999 ;;
}

{ .mmi
(p0)  and           r36 = r35, r34 ;;
(p0)  sub           r37 = r36, r33
      nop.i 999 ;;
}

{ .mmi
(p0)  setf.sig      f9  = r37
      nop.m 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fcvt.xf       f10 = f9
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p0)  fnorm.d       f8 = f10
(p0)  br.ret.sptk    b0 ;;
}

L(LOGB_NAN_INF): 

// X NAN or INFINITY, return f8 * f8
{ .mfb
      nop.m 999
(p0)  fma.d      f8= f8,f8,f0
(p0)  br.ret.sptk   b0 ;;
}

.endp logb#
ASM_SIZE_DIRECTIVE(logb)

// Stack operations when calling error support.
//       (1)               (2)                          (3) (call)              (4)
//   sp   -> +          psp -> +                     psp -> +                   sp -> +
//           |                 |                            |                         |
//           |                 | <- GR_Y               R3 ->| <- GR_RESULT            | -> f8
//           |                 |                            |                         |
//           | <-GR_Y      Y2->|                       Y2 ->| <- GR_Y                 |
//           |                 |                            |                         |
//           |                 | <- GR_X               X1 ->|                         |
//           |                 |                            |                         |
//  sp-64 -> +          sp ->  +                     sp ->  +                         +
//    save ar.pfs          save b0                                               restore gp
//    save gp                                                                    restore ar.pfs



.proc __libm_error_region
__libm_error_region:
L(LOGB_ZERO): 
.prologue

// f9  = |f8|
// f10 = -f9 = -|f8|
// f9  = 1.0/f10 = -1.0/-|f8|

{ .mfi 
      mov           r41 = 151      // Error code
(p0)  fmerge.s      f9 = f0,f8
      nop.i 999
}
;;


{ .mfi
      nop.m 999
      fmerge.ns     f10 = f0,f9
      nop.i 999
}
;;

// (1)
{ .mfi
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
        frcpa         f9,p6 = f1,f10
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs                 // Save ar.pfs
}
{ .mfi
.fframe 64
        add sp=-64,sp                          // Create new stack
        nop.f 0
        mov GR_SAVE_GP=gp                      // Save gp
};;


// (2)
{ .mmi
        stfd [GR_Parameter_Y] = f0,16         // STORE Parameter 2 on stack
        add GR_Parameter_X    = 16,sp         // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                     // Save b0
};;

.body
// (3)
{ .mib
        stfd [GR_Parameter_X] = f8            // STORE Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y    // Parameter 3 address
        nop.b 0                                         
}
{ .mib
        stfd [GR_Parameter_Y] = f9            // Store Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support# // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;

// (4)
{ .mmi
        ldfd  f8 = [GR_Parameter_RESULT]       // Get return result off stack
.restore sp
        add   sp = 64,sp                       // Restore stack pointer
        mov   b0 = GR_SAVE_B0                  // Restore return address
};;
{ .mib
        mov   gp = GR_SAVE_GP                  // Restore gp
        mov   ar.pfs = GR_SAVE_PFS             // Restore ar.pfs
        br.ret.sptk   b0
};;

.endp __libm_error_region
ASM_SIZE_DIRECTIVE(__libm_error_region)

.type   __libm_error_support#,@function
.global __libm_error_support#
