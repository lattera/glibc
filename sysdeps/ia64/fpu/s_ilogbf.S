.file "ilogbf.s"

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
// 2/03/00  Initial version
// 5/26/00  Fix bug when x a double-extended denormal; 
//          if x=0 call error routine, per C9X
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 1/20/01  Fixed result for x=0

.align 32
.global ilogbf#

.section .text
.proc  ilogbf#
.align 32

// API
//==============================================================
// int = ilogbf(float)

// Overview of operation
//==============================================================
// ilogbf computes log2(x) as an int
// and returns it in r8

// ilogbf is similar to logbf but differs in the  following ways:
//         +-inf
//            ilogbf: returns INT_MAX
//             logbf: returns +inf
//         Nan  returns FP_ILOGBNAN (which is either INT_MAX or INT_MIN)
//            ilogbf: returns INT_MAX (7fffffff)
//             logbf: returns QNAN (quieted SNAN)
//         0    returns FP_ILOGB0 (which is either INT_MIN or -INT_MAX)
//            ilogbf: returns INT_MIN (80000000)
//             logbf: returns -inf

// Registers used
//==============================================================

// general local registers: 
// ar.pfs r32
// r33 -> r37 
// r38 -> r41 used as parameters to error path

// predicate registers used: 
// p6 - x nan, inf
// p7 - x 0
// p8 - x norm, unorm
// p9 - x unorm

// floating-point registers used: 
// f8 - f10

#include "libm_support.h"

GR_SAVE_PFS         = r32
GR_SAVE_B0          = r34
GR_SAVE_GP          = r35
GR_Parameter_X      = r38
GR_Parameter_Y      = r39
GR_Parameter_RESULT = r40
GR_Parameter_TAG    = r41

FR_X                = f8
FR_Y                = f0
FR_RESULT           = f0


ilogbf: 

// Form signexp of 2^64 in case need to scale denormal
{ .mmf
      alloc          r32=ar.pfs,1,5,4,0
(p0)  mov      r37 = 0x1003f
(p0)  fnorm    f9 = f8 ;;
}

// Form 2^64 in case need to scale denormal
{ .mfi
(p0)  setf.exp f10 = r37
(p0)  fclass.m.unc p7, p8 = f8, 0xe3        
(p0)  mov      r34 = 0xffff ;;
}

// qnan snan inf norm     unorm 0 -+
// 1    1    1   0        0     0 11
// e                      3
// X ZERO, returns INT_MIN
// X INF or NAN, returns INT_MAX

{ .mfi
(p0)  mov      r35 = 0x1ffff
(p8)    fclass.m.unc p6, p8 = f8, 0x07
	nop.i 999 ;;
}
{ .mlx
	nop.m 999
(p7)    movl r8 = 0x000000007fffffff ;;       
}

{ .mib
	nop.m 999
	nop.i 999
(p6)    br.cond.spnt  L(ILOGB_ZERO) ;;
}

// Test for denormal
{ .mfi
	nop.m 999
(p8)    fclass.m.unc p9, p0 = f9, 0x0b        
	nop.i 999 ;;
}

L(ILOGB_COMMON):
// X NORMAL returns true exponent
{ .mmi
	nop.m 999 
(p8)    getf.exp r33 = f9
	nop.i 999 ;;
}

// If denormal add 64 to exponent bias for scaling
{ .mfb
(p9)    add     r34 = 64, r34   
	nop.f 999 
(p9)    br.cond.spnt  L(ILOGB_DENORM) ;;
}

{ .mmi
(p8)    and      r36 = r35, r33
	nop.m 999
	nop.i 999 ;;
}

{ .mib
(p8)    sub r8 = r36, r34                  
	nop.i 999
(p0)    br.ret.sptk    b0 ;;                     
}

L(ILOGB_DENORM):
// Here if x denormal
// Form x * 2^64 which is normal
// Return to common code
{ .mfb
        cmp.eq p8,p9 = r0,r0
        fmpy f9 = f9, f10
        br.cond.sptk  L(ILOGB_COMMON) ;;
}

// X ZERO
// return INT_MIN, call error support
L(ILOGB_ZERO): 
{.mlx
      mov            GR_Parameter_TAG = 158
(p6)  movl r33 = 0x0000000080000000 ;;
};;
.endp ilogbf
ASM_SIZE_DIRECTIVE(ilogbf)

.proc __libm_error_region
__libm_error_region:
.prologue
{ .mfi
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
        nop.f 0
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
        stfs [GR_Parameter_Y] = FR_Y,16         // Save Parameter 2 on stack
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
        stfs [GR_Parameter_Y] = FR_RESULT      // Store Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#  // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;
{ .mmi
        mov   r8 = r33                         // Store result
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
