.file "scalb.s"

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
// 1/26/01  Scalb completely reworked and now standalone version 
//
// API
//==============================================================
// double = scalb  (double x, double n) 
// input  floating point f8 and floating point f9
// output floating point f8
//
// Returns x* 2**n using an fma and detects overflow
// and underflow.   
//
//

#include "libm_support.h"

FR_Floating_X  = f8
FR_Result      = f8
FR_Floating_N  = f9
FR_Result2     = f9
FR_Norm_N      = f10
FR_Result3     = f11
FR_Norm_X      = f12
FR_N_float_int = f13
FR_Two_N       = f14
FR_Two_to_Big  = f15
FR_Big         = f6
FR_NBig        = f7

GR_N_Biased    = r15
GR_Big         = r16
GR_NBig        = r17
GR_Scratch     = r18
GR_Scratch1    = r19
GR_Bias        = r20
GR_N_as_int    = r21

GR_SAVE_B0          = r32
GR_SAVE_GP          = r33
GR_SAVE_PFS         = r34
GR_Parameter_X      = r35
GR_Parameter_Y      = r36
GR_Parameter_RESULT = r37
GR_Tag              = r38

.align 32
.global scalb

.section .text
.proc  scalb
.align 32

scalb: 
#ifdef _LIBC
.global __ieee754_scalb
.type __ieee754_scalb,@function
__ieee754_scalb:
#endif

//
//   Is x NAN, INF, ZERO, +-?
//
{    .mfi
     alloc          r32=ar.pfs,0,3,4,0
     fclass.m.unc  p7,p0 = FR_Floating_X, 0xe7 //@snan | @qnan | @inf | @zero
     addl  GR_Scratch  = 0x019C3F,r0 
}
//
//   Is y a NAN, INF, ZERO, +-?
//
{    .mfi
     nop.m 999
     fclass.m.unc  p6,p0 = FR_Floating_N, 0xe7 //@snan | @qnan | @inf |  @zero
     addl  GR_Scratch1  = 0x063BF,r0 
}
;;

//
//   Convert N to a fp integer
//   Normalize x
//
{    .mfi
     nop.m 0
     fnorm.s1  FR_Norm_N  =   FR_Floating_N 
     nop.i 999
}
{    .mfi
     nop.m 999
     fnorm.s1  FR_Norm_X  =   FR_Floating_X 
     nop.i 999
};;

//
//   Create 2*big
//   Create 2**-big 
//   Normalize x
//   Branch on special values.
//
{ .mib
     setf.exp      FR_Big = GR_Scratch                  
     nop.i 0 
(p6) br.cond.spnt  L(SCALB_NAN_INF_ZERO) 
}
{ .mib
     setf.exp      FR_NBig = GR_Scratch1                  
     nop.i 0 
(p7) br.cond.spnt  L(SCALB_NAN_INF_ZERO) 
};;

//
//   Convert N to a fp integer
//   Create -35000
//  
{    .mfi
     addl  GR_Scratch = 1,r0
     fcvt.fx.trunc.s1   FR_N_float_int = FR_Norm_N 
     addl    GR_NBig = -35000,r0
}
;;

//
//   Put N if a GP register
//   Convert  N_float_int to floating point value
//   Create 35000
//   Build the exponent Bias
//
{    .mii
     getf.sig     GR_N_as_int = FR_N_float_int
     shl   GR_Scratch = GR_Scratch,63
     addl  GR_Big = 35000,r0
}
{    .mfi
     addl GR_Bias = 0x0FFFF,r0
     fcvt.xf  FR_N_float_int = FR_N_float_int
     nop.i 0
};;

//
//   Catch those fp values that are beyond 2**64-1
//   Is N > 35000     
//   Is N < -35000     
//
{     .mfi
     cmp.ne.unc  p9,p10 = GR_N_as_int,GR_Scratch
     nop.f 0
     nop.i 0
}
{     .mmi
     cmp.ge.unc p6, p0 = GR_N_as_int, GR_Big
     cmp.le.unc p8, p0 = GR_N_as_int, GR_NBig
     nop.i 0
};;

//
//   Is N really an int, only for those non-int indefinites?
//   Create exp bias.     
//
{    .mfi
     add GR_N_Biased = GR_Bias,GR_N_as_int
(p9) fcmp.neq.unc.s1 p7,p0  =   FR_Norm_N, FR_N_float_int
     nop.i 0
};;

//
//   Branch and return if N is not an int.
//   Main path, create 2**N
//
{    .mfi
     setf.exp      FR_Two_N = GR_N_Biased                   
     nop.i                      999
}
{    .mfb
     nop.m 0
(p7) frcpa          f8,p11     =    f0,f0
(p7) br.ret.spnt    b0          
};;

//
//   Set denormal on denormal input x and denormal input N
//
{    .mfi
     nop.m                      999
(p10)fcmp.ge.s1    p6,p8 = FR_Norm_N,f0
     nop.i 0
};;
{    .mfi
     nop.m                      999
     fcmp.ge.s0    p0,p11 = FR_Floating_X,f0
     nop.i                      999
}
{    .mfi
     nop.m                      999
     fcmp.ge.s0    p12,p13 = FR_Floating_N,f0
     nop.i 0
};;

//
//   Adjust 2**N if N was very small or very large
//

{    .mfi
     nop.m 0
(p6) fma.s1  FR_Two_N = FR_Big,f1,f0
     nop.i 0
}
{ .mlx
     nop.m 999
(p0) movl GR_Scratch = 0x00000000000303FF 
};;
{    .mfi
     nop.m 0
(p8) fma.s1  FR_Two_N = FR_NBig,f1,f0
     nop.i 0
}
{    .mlx
     nop.m 999
(p0) movl GR_Scratch1= 0x00000000000103FF 
};;

//   Set up necessary status fields 
//
//   S0 user supplied status
//   S2 user supplied status + WRE + TD  (Overflows)
//   S3 user supplied status + FZ + TD   (Underflows)
//
{    .mfi
     nop.m 999
(p0) fsetc.s3 0x7F,0x41
     nop.i 999
}
{    .mfi
     nop.m 999
(p0) fsetc.s2 0x7F,0x42
     nop.i 999
};;

//
//   Do final operation
//
{    .mfi
     setf.exp FR_NBig = GR_Scratch
     fma.d.s0     FR_Result = FR_Two_N,FR_Norm_X,f0 
     nop.i                           999
}
{    .mfi
     nop.m                           999
     fma.d.s3     FR_Result3 = FR_Two_N,FR_Norm_X,f0 
     nop.i                           999
};;
{    .mfi
     setf.exp FR_Big = GR_Scratch1
     fma.d.s2     FR_Result2 = FR_Two_N,FR_Norm_X,f0 
     nop.i                           999
};;

//   Check for overflow or underflow.
//
//   S0 user supplied status
//   S2 user supplied status + WRE + TD  (Overflow)
//   S3 user supplied status + FZ + TD   (Underflow)
//
//
//   Restore s3
//   Restore s2
//
{    .mfi
     nop.m 0
     fsetc.s3 0x7F,0x40
     nop.i 999 
}
{    .mfi
     nop.m 0
     fsetc.s2 0x7F,0x40
     nop.i 999
};;

//
//   Is the result zero?
//
{    .mfi
     nop.m 999
     fclass.m.unc   p6, p0 =  FR_Result3, 0x007
     nop.i 999 
} 
{    .mfi
     addl GR_Tag = 53, r0
     fcmp.ge.unc.s1 p7, p8 = FR_Result2 , FR_Big
     nop.i 0
};;

//
//   Detect masked underflow - Tiny + Inexact Only
//
{    .mfi
     nop.m 999
(p6) fcmp.neq.unc.s1 p6, p0 = FR_Result , FR_Result2
     nop.i 999 
};; 

//
//   Is result bigger the allowed range?
//   Branch out for underflow
//
{    .mfb
(p6) addl GR_Tag = 54, r0
(p8) fcmp.le.unc.s1 p9, p10 = FR_Result2 , FR_NBig
(p6) br.cond.spnt L(SCALB_UNDERFLOW) 
};;

//
//   Branch out for overflow
//
{ .mbb
     nop.m 0
(p7) br.cond.spnt L(SCALB_OVERFLOW) 
(p9) br.cond.spnt L(SCALB_OVERFLOW) 
};;

//
//   Return from main path.
//
{    .mfb
     nop.m 999
     nop.f 0
     br.ret.sptk     b0;;                   
}

L(SCALB_NAN_INF_ZERO): 

//
//   Convert N to a fp integer
//  
{    .mfi
     addl  GR_Scratch = 1,r0
     fcvt.fx.trunc.s1  FR_N_float_int = FR_Norm_N 
     nop.i 999
}
{    .mfi
     nop.m 0
     fclass.m.unc  p6,p0 = FR_Floating_N, 0xc3 //@snan | @qnan 
     nop.i 0
};;
{    .mfi
     nop.m 0
     fclass.m.unc  p7,p0 = FR_Floating_X, 0xc3 //@snan | @qnan 
     shl   GR_Scratch = GR_Scratch,63
};;
{    .mfi
     nop.m 0
     fclass.m.unc  p8,p0 = FR_Floating_N, 0x21 // @inf
     nop.i 0
}
  {  .mfi
     nop.m 0
     fclass.m.unc  p9,p0 = FR_Floating_N, 0x22 // @-inf
     nop.i 0
};;

//
//   Either X or N is a Nan, return result and possible raise invalid.
//
{    .mfb
     nop.m 0
(p6) fma.d.s0     FR_Result = FR_Floating_N,FR_Floating_X,f0 
(p6) br.ret.spnt  b0
};;
{    .mfb
     getf.sig     GR_N_as_int = FR_N_float_int
(p7) fma.d.s0     FR_Result = FR_Floating_N,FR_Floating_X,f0 
(p7) br.ret.spnt  b0
};;

//
//   If N + Inf do something special
//   For N = -Inf, create Int
//
{    .mfb
     nop.m 0
(p8) fma.d.s0    FR_Result = FR_Floating_X, FR_Floating_N,f0 
(p8) br.ret.spnt   b0
}
{    .mfi
     nop.m 0
(p9) fnma.d.s0   FR_Floating_N = FR_Floating_N, f1, f0 
     nop.i 0
};;

//
//   If N==-Inf,return x/(-N)
//
{    .mfb
     nop.m 0
(p9) frcpa.s0        FR_Result,p6 =  FR_Floating_X,FR_Floating_N
(p9) br.ret.spnt    b0          
};;

//
//   Convert  N_float_int to floating point value
//
{     .mfi
     cmp.ne.unc  p9,p0     =   GR_N_as_int,GR_Scratch
     fcvt.xf  FR_N_float_int = FR_N_float_int
     nop.i  0
};;

//
//   Is N an integer.
//
{    .mfi
     nop.m 0
(p9) fcmp.neq.unc.s1 p7,p0  =   FR_Norm_N, FR_N_float_int
     nop.i 0
};;

//
//   If N not an int, return NaN and raise invalid.
//
{    .mfb
     nop.m 0
(p7) frcpa.s0        FR_Result,p6     =    f0,f0
(p7) br.ret.spnt    b0          
};;

//
//   Always return x in other path. 
//
{    .mfb
     nop.m 0
     fma.d.s0      FR_Result = FR_Floating_X,f1,f0 
     br.ret.sptk   b0
};;

.endp scalb
ASM_SIZE_DIRECTIVE(scalb)
#ifdef _LIBC
ASM_SIZE_DIRECTIVE(__ieee754_scalb)
#endif
.proc __libm_error_region
__libm_error_region:

L(SCALB_OVERFLOW): 
L(SCALB_UNDERFLOW): 

//
// Get stack address of N
//
.prologue
{ .mfi
    add   GR_Parameter_Y=-32,sp         
    nop.f 0
.save   ar.pfs,GR_SAVE_PFS
    mov  GR_SAVE_PFS=ar.pfs              
}
//
// Adjust sp 
//
{ .mfi
.fframe 64
   add sp=-64,sp                         
   nop.f 0
   mov GR_SAVE_GP=gp       
};;

//
//  Store N on stack in correct position 
//  Locate the address of x on stack
//
{ .mmi
   stfd [GR_Parameter_Y] = FR_Norm_N,16       
   add GR_Parameter_X = 16,sp          
.save   b0, GR_SAVE_B0
   mov GR_SAVE_B0=b0                  
};;

//
// Store x on the stack.
// Get address for result on stack.
//
.body
{ .mib
   stfd [GR_Parameter_X] = FR_Norm_X 
   add   GR_Parameter_RESULT = 0,GR_Parameter_Y   
   nop.b 0
}
{ .mib
   stfd [GR_Parameter_Y] = FR_Result                 
   add   GR_Parameter_Y = -16,GR_Parameter_Y
   br.call.sptk b0=__libm_error_support#   
};;

//
//  Get location of result on stack
//
{ .mmi
   nop.m 0
   nop.m 0
   add   GR_Parameter_RESULT = 48,sp    
};;

//
//  Get the new result 
//
{ .mmi
   ldfd  FR_Result = [GR_Parameter_RESULT]      
.restore sp
   add   sp = 64,sp                       
   mov   b0 = GR_SAVE_B0                  
};;

//
//  Restore gp, ar.pfs and return
//
{ .mib
   mov   gp = GR_SAVE_GP                  
   mov   ar.pfs = GR_SAVE_PFS             
   br.ret.sptk     b0                  
};;

.endp __libm_error_region
ASM_SIZE_DIRECTIVE(__libm_error_region)

.type   __libm_error_support#,@function
.global __libm_error_support#
