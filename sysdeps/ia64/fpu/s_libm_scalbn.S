.file "libm_scalbn.s"


// Copyright (c) 2000 - 2003, Intel Corporation
// All rights reserved.
//
// Contributed 2000 by the Intel Numerics Group, Intel Corporation
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// * The name of Intel Corporation may not be used to endorse or promote
// products derived from this software without specific prior written
// permission.

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
// http://www.intel.com/software/products/opensource/libraries/num.htm.
//
// History
//==============================================================
// 02/02/00 Initial version
// 01/26/01 Scalbn completely reworked and now standalone version 
// 01/04/02 Added handling for int 32 or 64 bits
// 05/20/02 Cleaned up namespace and sf0 syntax
// 02/10/03 Reordered header: .section, .global, .proc, .align
//
// API
//==============================================================
// double __libm_scalbn  (double x, int n, int int_type) 
// input  floating point f8 and int n (r33), int int_type (r34)
// output floating point f8
//
// int_type = 0 if int is 32 bits
// int_type = 1 if int is 64 bits
//
// Returns x* 2**n using an fma and detects overflow
// and underflow.   
//
//

FR_Big         = f6
FR_NBig        = f7
FR_Floating_X  = f8
FR_Result      = f8
FR_Result2     = f9
FR_Result3     = f11
FR_Norm_X      = f12
FR_Two_N       = f14
FR_Two_to_Big  = f15

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

.section .text
GLOBAL_LIBM_ENTRY(__libm_scalbn)

//
//   Is x NAN, INF, ZERO, +-?
//   Build the exponent Bias
//
{    .mfi
     alloc         r32=ar.pfs,3,0,4,0
     fclass.m      p7,p0 = FR_Floating_X, 0xe7 //@snan | @qnan | @inf | @zero
     addl          GR_Bias = 0x0FFFF,r0
}

//
//   Is N zero?
//   Normalize x
//   Is integer type 32 bits?
//
{    .mfi
     cmp.eq        p6,p0 = r33,r0  
     fnorm.s1      FR_Norm_X  =   FR_Floating_X 
     cmp.eq        p8,p9 = r34,r0  
}
;;

//   Sign extend N if int is 32 bits
{    .mfi
(p9) mov           GR_N_as_int = r33     // Copy N if int is 64 bits
     nop.f 0
(p8) sxt4          GR_N_as_int = r33     // Sign extend N if int is 32 bits
}
;;

//
//   Branch and return special values.
//   Create -35000
//   Create 35000
//
{    .mfi
     addl          GR_Big = 35000,r0
     nop.f         0
     add           GR_N_Biased = GR_Bias,GR_N_as_int
}
{    .mfb
     addl          GR_NBig = -35000,r0
(p7) fma.d.s0      FR_Result = FR_Floating_X,f1, f0 
(p7) br.ret.spnt   b0  
};;

//
//   Build the exponent Bias
//   Return x when N = 0
//
{    .mfi
     setf.exp      FR_Two_N = GR_N_Biased                   
     nop.f         0
     addl          GR_Scratch1  = 0x063BF,r0 
}
{    .mfb
     addl          GR_Scratch  = 0x019C3F,r0 
(p6) fma.d.s0      FR_Result = FR_Floating_X,f1, f0 
(p6) br.ret.spnt   b0  
};;

//
//   Create 2*big
//   Create 2**-big 
//   Is N > 35000     
//   Is N < -35000     
//   Raise Denormal operand flag with compare
//   Main path, create 2**N
//
{    .mfi
     setf.exp      FR_NBig = GR_Scratch1                  
     nop.f         0
     cmp.ge        p6, p0 = GR_N_as_int, GR_Big
}
{    .mfi
     setf.exp      FR_Big = GR_Scratch                  
     fcmp.ge.s0    p0,p11 = FR_Floating_X,f0
     cmp.le        p8, p0 = GR_N_as_int, GR_NBig
};;

//
//   Adjust 2**N if N was very small or very large
//
{    .mfi
     nop.m 0
(p6) fma.s1        FR_Two_N = FR_Big,f1,f0
     nop.i 0
}
{ .mlx
     nop.m 999
     movl          GR_Scratch = 0x00000000000303FF 
};;


{    .mfi
     nop.m 0
(p8) fma.s1        FR_Two_N = FR_NBig,f1,f0
     nop.i 0
}
{    .mlx
     nop.m 999
     movl          GR_Scratch1= 0x00000000000103FF 
};;

//   Set up necessary status fields 
//
//   S0 user supplied status
//   S2 user supplied status + WRE + TD  (Overflows)
//   S3 user supplied status + FZ + TD   (Underflows)
//
{    .mfi
     nop.m 999
     fsetc.s3      0x7F,0x41
     nop.i 999
}
{    .mfi
     nop.m 999
     fsetc.s2      0x7F,0x42
     nop.i 999
};;

//
//   Do final operation
//
{    .mfi
     setf.exp      FR_NBig = GR_Scratch
     fma.d.s0      FR_Result = FR_Two_N,FR_Norm_X,f0 
     nop.i         999
}
{    .mfi
     nop.m         999
     fma.d.s3      FR_Result3 = FR_Two_N,FR_Norm_X,f0 
     nop.i         999
};;
{    .mfi
     setf.exp      FR_Big = GR_Scratch1
     fma.d.s2      FR_Result2 = FR_Two_N,FR_Norm_X,f0 
     nop.i         999
};;

//   Check for overflow or underflow.
//   Restore s3
//   Restore s2
//
{    .mfi
     nop.m 0
     fsetc.s3      0x7F,0x40
     nop.i 999 
}
{    .mfi
     nop.m 0
     fsetc.s2      0x7F,0x40
     nop.i 999
};;

//
//   Is the result zero?
//
{    .mfi
     nop.m 999
     fclass.m      p6, p0 =  FR_Result3, 0x007
     nop.i 999 
} 
{    .mfi
     addl          GR_Tag = 176, r0
     fcmp.ge.s1    p7, p8 = FR_Result2 , FR_Big
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
(p6) addl           GR_Tag = 177, r0
(p8) fcmp.le.unc.s1 p9, p10 = FR_Result2 , FR_NBig
(p6) br.cond.spnt   SCALBN_UNDERFLOW 
};;

//
//   Branch out for overflow
//
{ .mbb
     nop.m 0
(p7) br.cond.spnt   SCALBN_OVERFLOW 
(p9) br.cond.spnt   SCALBN_OVERFLOW 
};;

//
//   Return from main path.
//
{    .mfb
     nop.m 999
     nop.f 0
     br.ret.sptk     b0;;                   
}

GLOBAL_LIBM_END(__libm_scalbn)
__libm_error_region:

SCALBN_OVERFLOW: 
SCALBN_UNDERFLOW: 

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
   st8 [GR_Parameter_Y] =  GR_N_as_int,16       
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

LOCAL_LIBM_END(__libm_error_region)

.type   __libm_error_support#,@function
.global __libm_error_support#
