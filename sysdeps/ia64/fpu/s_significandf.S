.file "significandf.s"

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
// 2/03/00: Modified to improve speed
// 5/31/00: Fixed bug when x a double-extended denormal
//
// API
//==============================================================
// float significandf(float x)
// Overview of operation
//==============================================================
// If x = sig * 2**n with 1 <= sig < 2
// significandf returns sig
//
// predicate registers used: 
// p6, p7
//
// floating-point registers used:  
// f8, f9, f10

#include "libm_support.h"

.align 32
.global significandf#

.section .text
.proc  significandf#
.align 32

significandf: 

// qnan snan inf norm     unorm 0 -+
// 1    1    1   0        0     1 11

// f10 gets f8(sign) with f1(exp,significand)
{ .mfi
      nop.m 999
(p0)  fmerge.s       f10 = f8,f1               
      nop.i 999
}
{ .mfi
      nop.m 999
(p0)  fnorm          f9  = f8                  
      nop.i 999 ;;
}

// Test for denormal input
{ .mfi
      nop.m 999
(p0)  fclass.m.unc   p7,p0 = f8, 0x0b
      nop.i 999 ;;
}

// p6 = TRUE ==> x is not (nan,inf,0)
//               return sign(f8) exp(f1) significand(f8)
// else          x is (nan,inf,0)
//               return sign(f8) exp(f8) significand(f8), normalized.
{ .mfi
      nop.m 999
(p0)  fclass.m.unc   p0,p6 = f8, 0xe7          
      nop.i 999 ;;
}

{ .mmb
      nop.m 999
      nop.m 999
(p7)  br.cond.spnt L(SIGNIFICAND_DENORM) ;; // Branch if x denormal
}

{ .mfi
      nop.m 999
(p6)  fmerge.se      f8 = f10,f8
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p0)  fnorm.s        f8 = f8
(p0)  br.ret.sptk    b0 ;;
}

L(SIGNIFICAND_DENORM):
// Here if x denorm
{ .mfi
      nop.m 999
(p0)  fmerge.se      f8 = f10,f9
      nop.i 999 ;;
}

// Check if fnorm(x) still denormal, means x double-extended denormal
{ .mfi
      nop.m 999
(p0)  fclass.m.unc   p7,p0 = f9, 0x0b
      nop.i 999 ;;
}

// This will be the final result unless x double-extended denormal
{ .mfi
      nop.m 999
(p0)  fnorm.s        f8 = f8
      nop.i 999 ;;
}

// If x double-extended denorm, then significand ok, but must merge in
//    correct signexp
{ .mfi
      nop.m 999
(p7)  fmerge.se      f8 = f10,f8
      nop.i 999 ;;
}

// Final normalization if x double-extended denorm
{ .mfb
      nop.m 999
(p7)  fnorm.s        f8 = f8
(p0)  br.ret.sptk    b0 ;;
}

.endp significandf
ASM_SIZE_DIRECTIVE(significandf)
