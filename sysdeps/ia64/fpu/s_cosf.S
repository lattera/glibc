
.file "sincosf.s"


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


// History
//==============================================================
// 2/02/00  Initial revision 
// 4/02/00  Unwind support added.
// 5/10/00  Improved speed with new algorithm.
// 8/08/00  Improved speed by avoiding SIR flush.
// 8/17/00  Changed predicate register macro-usage to direct predicate
//          names due to an assembler bug.
// 8/30/00  Put sin_of_r before sin_tbl_S_cos_of_r to gain a cycle 
// 1/02/00  Fixed flag settings, improved speed.
//
// API
//==============================================================
// float sinf( float x);
// float cosf( float x);
//

#include "libm_support.h"	
	
// Assembly macros
//==============================================================

// SIN_Sin_Flag               = p6
// SIN_Cos_Flag               = p7

// integer registers used

 SIN_AD_PQ_1                = r33
 SIN_AD_PQ_2                = r33
 sin_GR_sincos_flag         = r34
 sin_GR_Mint                = r35

 sin_GR_index               = r36
 gr_tmp                     = r37

 GR_SAVE_B0                 = r37
 GR_SAVE_GP                 = r38
 GR_SAVE_PFS                = r39


// floating point registers used

 sin_coeff_P1               = f32
 sin_coeff_P2               = f33
 sin_coeff_Q1               = f34
 sin_coeff_Q2               = f35
 sin_coeff_P4               = f36
 sin_coeff_P5               = f37
 sin_coeff_Q3               = f38
 sin_coeff_Q4               = f39
 sin_Mx                     = f40
 sin_Mfloat                 = f41
 sin_tbl_S                  = f42
 sin_tbl_C                  = f43
 sin_r                      = f44
 sin_rcube                  = f45
 sin_tsq                    = f46
 sin_r7                     = f47
 sin_t                      = f48
 sin_poly_p2                = f49
 sin_poly_p1                = f50
 fp_tmp                     = f51
 sin_poly_p3                = f52
 sin_poly_p4                = f53
 sin_of_r                   = f54
 sin_S_t                    = f55
 sin_poly_q2                = f56
 sin_poly_q1                = f57
 sin_S_tcube                = f58
 sin_poly_q3                = f59
 sin_poly_q4                = f60
 sin_tbl_S_tcube            = f61
 sin_tbl_S_cos_of_r         = f62

 sin_coeff_Q5               = f63
 sin_coeff_Q6               = f64
 sin_coeff_P3               = f65

 sin_poly_q5                = f66
 sin_poly_q12               = f67
 sin_poly_q3456             = f68
 fp_tmp2                    = f69
 SIN_NORM_f8                = f70


#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

sin_coeff_1_table:
ASM_TYPE_DIRECTIVE(sin_coeff_1_table,@object)
data8 0xBF56C16C16BF6462       // q3
data8 0x3EFA01A0128B9EBC       // q4
data8 0xBE927E42FDF33FFE       // q5
data8 0x3E21DA5C72A446F3       // q6
data8 0x3EC71DD1D5E421A4       // p4
data8 0xBE5AC5C9D0ACF95A       // p5
data8 0xBFC55555555554CA       // p1
data8 0x3F811111110F2395       // p2
data8 0xBFE0000000000000       // q1
data8 0x3FA55555555554EF       // q2
data8 0xBF2A01A011232913       // p3
data8 0x0000000000000000       // pad
 

/////////////////////////////////////////

data8 0xBFE1A54991426566   //sin(-32)
data8 0x3FEAB1F5305DE8E5   //cos(-32)
data8 0x3FD9DBC0B640FC81   //sin(-31)
data8 0x3FED4591C3E12A20   //cos(-31)
data8 0x3FEF9DF47F1C903D   //sin(-30)
data8 0x3FC3BE82F2505A52   //cos(-30)
data8 0x3FE53C7D20A6C9E7   //sin(-29)
data8 0xBFE7F01658314E47   //cos(-29)
data8 0xBFD156853B4514D6   //sin(-28)
data8 0xBFEECDAAD1582500   //cos(-28)
data8 0xBFEE9AA1B0E5BA30   //sin(-27)
data8 0xBFD2B266F959DED5   //cos(-27)
data8 0xBFE866E0FAC32583   //sin(-26)
data8 0x3FE4B3902691A9ED   //cos(-26)
data8 0x3FC0F0E6F31E809D   //sin(-25)
data8 0x3FEFB7EEF59504FF   //cos(-25)
data8 0x3FECFA7F7919140F   //sin(-24)
data8 0x3FDB25BFB50A609A   //cos(-24)
data8 0x3FEB143CD0247D02   //sin(-23)
data8 0xBFE10CF7D591F272   //cos(-23)
data8 0x3F8220A29F6EB9F4   //sin(-22)
data8 0xBFEFFFADD8D4ACDA   //cos(-22)
data8 0xBFEAC5E20BB0D7ED   //sin(-21)
data8 0xBFE186FF83773759   //cos(-21)
data8 0xBFED36D8F55D3CE0   //sin(-20)
data8 0x3FDA1E043964A83F   //cos(-20)
data8 0xBFC32F2D28F584CF   //sin(-19)
data8 0x3FEFA377DE108258   //cos(-19)
data8 0x3FE8081668131E26   //sin(-18)
data8 0x3FE52150815D2470   //cos(-18)
data8 0x3FEEC3C4AC42882B   //sin(-17)
data8 0xBFD19C46B07F58E7   //cos(-17)
data8 0x3FD26D02085F20F8   //sin(-16)
data8 0xBFEEA5257E962F74   //cos(-16)
data8 0xBFE4CF2871CEC2E8   //sin(-15)
data8 0xBFE84F5D069CA4F3   //cos(-15)
data8 0xBFEFB30E327C5E45   //sin(-14)
data8 0x3FC1809AEC2CA0ED   //cos(-14)
data8 0xBFDAE4044881C506   //sin(-13)
data8 0x3FED09CDD5260CB7   //cos(-13)
data8 0x3FE12B9AF7D765A5   //sin(-12)
data8 0x3FEB00DA046B65E3   //cos(-12)
data8 0x3FEFFFEB762E93EB   //sin(-11)
data8 0x3F7220AE41EE2FDF   //cos(-11)
data8 0x3FE1689EF5F34F52   //sin(-10)
data8 0xBFEAD9AC890C6B1F   //cos(-10)
data8 0xBFDA6026360C2F91   //sin( -9)
data8 0xBFED27FAA6A6196B   //cos( -9)
data8 0xBFEFA8D2A028CF7B   //sin( -8)
data8 0xBFC29FBEBF632F94   //cos( -8)
data8 0xBFE50608C26D0A08   //sin( -7)
data8 0x3FE81FF79ED92017   //cos( -7)
data8 0x3FD1E1F18AB0A2C0   //sin( -6)
data8 0x3FEEB9B7097822F5   //cos( -6)
data8 0x3FEEAF81F5E09933   //sin( -5)
data8 0x3FD22785706B4AD9   //cos( -5)
data8 0x3FE837B9DDDC1EAE   //sin( -4)
data8 0xBFE4EAA606DB24C1   //cos( -4)
data8 0xBFC210386DB6D55B   //sin( -3)
data8 0xBFEFAE04BE85E5D2   //cos( -3)
data8 0xBFED18F6EAD1B446   //sin( -2)
data8 0xBFDAA22657537205   //cos( -2)
data8 0xBFEAED548F090CEE   //sin( -1)
data8 0x3FE14A280FB5068C   //cos( -1)
data8 0x0000000000000000   //sin(  0)
data8 0x3FF0000000000000   //cos(  0)
data8 0x3FEAED548F090CEE   //sin(  1)
data8 0x3FE14A280FB5068C   //cos(  1)
data8 0x3FED18F6EAD1B446   //sin(  2)
data8 0xBFDAA22657537205   //cos(  2)
data8 0x3FC210386DB6D55B   //sin(  3)
data8 0xBFEFAE04BE85E5D2   //cos(  3)
data8 0xBFE837B9DDDC1EAE   //sin(  4)
data8 0xBFE4EAA606DB24C1   //cos(  4)
data8 0xBFEEAF81F5E09933   //sin(  5)
data8 0x3FD22785706B4AD9   //cos(  5)
data8 0xBFD1E1F18AB0A2C0   //sin(  6)
data8 0x3FEEB9B7097822F5   //cos(  6)
data8 0x3FE50608C26D0A08   //sin(  7)
data8 0x3FE81FF79ED92017   //cos(  7)
data8 0x3FEFA8D2A028CF7B   //sin(  8)
data8 0xBFC29FBEBF632F94   //cos(  8)
data8 0x3FDA6026360C2F91   //sin(  9)
data8 0xBFED27FAA6A6196B   //cos(  9)
data8 0xBFE1689EF5F34F52   //sin( 10)
data8 0xBFEAD9AC890C6B1F   //cos( 10)
data8 0xBFEFFFEB762E93EB   //sin( 11)
data8 0x3F7220AE41EE2FDF   //cos( 11)
data8 0xBFE12B9AF7D765A5   //sin( 12)
data8 0x3FEB00DA046B65E3   //cos( 12)
data8 0x3FDAE4044881C506   //sin( 13)
data8 0x3FED09CDD5260CB7   //cos( 13)
data8 0x3FEFB30E327C5E45   //sin( 14)
data8 0x3FC1809AEC2CA0ED   //cos( 14)
data8 0x3FE4CF2871CEC2E8   //sin( 15)
data8 0xBFE84F5D069CA4F3   //cos( 15)
data8 0xBFD26D02085F20F8   //sin( 16)
data8 0xBFEEA5257E962F74   //cos( 16)
data8 0xBFEEC3C4AC42882B   //sin( 17)
data8 0xBFD19C46B07F58E7   //cos( 17)
data8 0xBFE8081668131E26   //sin( 18)
data8 0x3FE52150815D2470   //cos( 18)
data8 0x3FC32F2D28F584CF   //sin( 19)
data8 0x3FEFA377DE108258   //cos( 19)
data8 0x3FED36D8F55D3CE0   //sin( 20)
data8 0x3FDA1E043964A83F   //cos( 20)
data8 0x3FEAC5E20BB0D7ED   //sin( 21)
data8 0xBFE186FF83773759   //cos( 21)
data8 0xBF8220A29F6EB9F4   //sin( 22)
data8 0xBFEFFFADD8D4ACDA   //cos( 22)
data8 0xBFEB143CD0247D02   //sin( 23)
data8 0xBFE10CF7D591F272   //cos( 23)
data8 0xBFECFA7F7919140F   //sin( 24)
data8 0x3FDB25BFB50A609A   //cos( 24)
data8 0xBFC0F0E6F31E809D   //sin( 25)
data8 0x3FEFB7EEF59504FF   //cos( 25)
data8 0x3FE866E0FAC32583   //sin( 26)
data8 0x3FE4B3902691A9ED   //cos( 26)
data8 0x3FEE9AA1B0E5BA30   //sin( 27)
data8 0xBFD2B266F959DED5   //cos( 27)
data8 0x3FD156853B4514D6   //sin( 28)
data8 0xBFEECDAAD1582500   //cos( 28)
data8 0xBFE53C7D20A6C9E7   //sin( 29)
data8 0xBFE7F01658314E47   //cos( 29)
data8 0xBFEF9DF47F1C903D   //sin( 30)
data8 0x3FC3BE82F2505A52   //cos( 30)
data8 0xBFD9DBC0B640FC81   //sin( 31)
data8 0x3FED4591C3E12A20   //cos( 31)
data8 0x3FE1A54991426566   //sin( 32)
data8 0x3FEAB1F5305DE8E5   //cos( 32)
ASM_SIZE_DIRECTIVE(sin_coeff_1_table)

//////////////////////////////////////////


.global sinf
.global cosf
#ifdef _LIBC
.global __sinf
.global __cosf
#endif

.text
.proc cosf
#ifdef _LIBC
.proc __cosf
#endif
.align 32


cosf:
#ifdef _LIBC
__cosf:
#endif
{ .mfi
     alloc          r32                      = ar.pfs,1,7,0,0
     fcvt.fx.s1     sin_Mx                   =    f8
     cmp.ne    p6,p7     =    r0,r0        // p7 set if cos
}
{ .mfi
     addl           SIN_AD_PQ_1              =    @ltoff(sin_coeff_1_table),gp
     fnorm.s0 SIN_NORM_f8 = f8        // Sets denormal or invalid
     mov sin_GR_sincos_flag = 0x0
}
;;

{ .mfi
     ld8       SIN_AD_PQ_1    =    [SIN_AD_PQ_1]
     fclass.m.unc  p9,p0      =    f8, 0x07
     cmp.ne p8,p0 = r0,r0
}
{ .mfb
     nop.m 999
     nop.f 999
     br.sptk L(SINCOSF_COMMON)
}
;;

.endp cosf
ASM_SIZE_DIRECTIVE(cosf)


.text
.proc  sinf
#ifdef _LIBC
.proc  __sinf
#endif
.align 32

sinf:
#ifdef _LIBC
__sinf:	
#endif
{ .mfi
     alloc          r32                      = ar.pfs,1,7,0,0
     fcvt.fx.s1     sin_Mx                   =    f8
     cmp.eq    p6,p7     =    r0,r0        // p6 set if sin
}
{ .mfi
     addl           SIN_AD_PQ_1              =    @ltoff(sin_coeff_1_table),gp
     fnorm.s0 SIN_NORM_f8 = f8        // Sets denormal or invalid
     mov sin_GR_sincos_flag = 0x1
}
;;

{ .mfi
     ld8       SIN_AD_PQ_1    =    [SIN_AD_PQ_1]
     fclass.m.unc  p8,p0      =    f8, 0x07
     cmp.ne p9,p0 = r0,r0
}
{ .mfb
     nop.m 999
     nop.f 999
     br.sptk L(SINCOSF_COMMON)
}
;;


L(SINCOSF_COMMON):

// Here with p6 if sin, p7 if cos, p8 if sin(0), p9 if cos(0)


{ .mmf
     ldfpd      sin_coeff_Q3, sin_coeff_Q4     = [SIN_AD_PQ_1], 16
     nop.m 999
     fclass.m.unc  p11,p0      =    f8, 0x23	// Test for x=inf
}
;;

{ .mfb
     ldfpd      sin_coeff_Q5, sin_coeff_Q6     = [SIN_AD_PQ_1], 16
     fclass.m.unc  p10,p0      =    f8, 0xc3	// Test for x=nan
(p8) br.ret.spnt b0                   // Exit for sin(0)
}
{ .mfb
     nop.m 999
(p9) fma.s      f8 = f1,f1,f0
(p9) br.ret.spnt b0                   // Exit for cos(0)
}
;;

{ .mmf
     ldfpd      sin_coeff_P4, sin_coeff_P5     = [SIN_AD_PQ_1], 16
     addl gr_tmp = -1,r0
     fcvt.xf    sin_Mfloat                     =    sin_Mx
}
;;

{     .mfi
     getf.sig  sin_GR_Mint    =    sin_Mx
(p11) frcpa.s0      f8,p13      =    f0,f0  // qnan indef if x=inf
     nop.i 999
}
{     .mfb
     ldfpd      sin_coeff_P1, sin_coeff_P2     = [SIN_AD_PQ_1], 16
     nop.f 999
(p11) br.ret.spnt b0                   // Exit for x=inf
}
;;

{     .mfi
     ldfpd      sin_coeff_Q1, sin_coeff_Q2     = [SIN_AD_PQ_1], 16
     nop.f                      999
     cmp.ge    p8,p9          = -33,sin_GR_Mint
}
{     .mfb
     add       sin_GR_index   =    32,sin_GR_Mint
(p10) fma.s      f8 = f8,f1,f0         // Force qnan if x=nan
(p10) br.ret.spnt b0                   // Exit for x=nan
}
;;

{ .mmi
     ldfd      sin_coeff_P3   = [SIN_AD_PQ_1], 16
(p9) cmp.le    p8,p0        = 33, sin_GR_Mint 
     shl       sin_GR_index   =    sin_GR_index,4
}
;;


{     .mfi
     setf.sig fp_tmp = gr_tmp  // Create constant such that fmpy sets inexact
     fnma.s1   sin_r     =    f1,sin_Mfloat,SIN_NORM_f8
(p8) cmp.eq.unc p11,p12=sin_GR_sincos_flag,r0  // p11 if must call dbl cos
                                               // p12 if must call dbl sin
}
{    .mbb
     add       SIN_AD_PQ_2    =    sin_GR_index,SIN_AD_PQ_1
(p11) br.cond.spnt COS_DOUBLE
(p12) br.cond.spnt SIN_DOUBLE
}
;;

.pred.rel "mutex",p6,p7    //SIN_Sin_Flag, SIN_Cos_Flag
{     .mmi
(p6) ldfpd     sin_tbl_S,sin_tbl_C =    [SIN_AD_PQ_2]
(p7) ldfpd     sin_tbl_C,sin_tbl_S =    [SIN_AD_PQ_2]
               nop.i                           999
}
;;

{     .mfi
     nop.m                 999
(p6) fclass.m.unc p8,p0 = f8, 0x0b // If sin, note denormal input to set uflow
     nop.i                 999
}
{     .mfi
     nop.m                 999
     fma.s1    sin_t     =    sin_r,sin_r,f0
     nop.i                 999
}
;;

{     .mfi
     nop.m                 999
     fma.s1    sin_rcube =    sin_t,sin_r,f0
     nop.i                 999
}
{     .mfi
     nop.m                 999
     fma.s1    sin_tsq   =    sin_t,sin_t,f0
     nop.i                 999
}
;;

{     .mfi
     nop.m                      999
     fma.s1    sin_poly_q3    =    sin_t,sin_coeff_Q4,sin_coeff_Q3
     nop.i                      999
}
{     .mfi
     nop.m                      999
     fma.s1    sin_poly_q5    =    sin_t,sin_coeff_Q6,sin_coeff_Q5
     nop.i                      999
}
;;

{     .mfi
     nop.m                      999
     fma.s1    sin_poly_p1    =    sin_t,sin_coeff_P5,sin_coeff_P4
     nop.i                      999
}
{     .mfi
     nop.m                      999
     fma.s1    sin_poly_p2    =    sin_t,sin_coeff_P2,sin_coeff_P1
     nop.i                      999
}
;;

{     .mfi
     nop.m                      999
     fma.s1    sin_poly_q1    =    sin_t,sin_coeff_Q2,sin_coeff_Q1
     nop.i                      999
}
{     .mfi
     nop.m                      999
     fma.s1    sin_S_t   =    sin_t,sin_tbl_S,f0
     nop.i                      999
}
;;

{     .mfi
     nop.m                 999
(p8) fmpy.s.s0 fp_tmp2 = f8,f8  // Dummy mult to set underflow if sin(denormal)
     nop.i                 999
}
{     .mfi
     nop.m                 999
     fma.s1    sin_r7    =    sin_rcube,sin_tsq,f0
     nop.i                 999
}
;;

{     .mfi
     nop.m                      999
     fma.s1    sin_poly_q3456 =    sin_tsq,sin_poly_q5,sin_poly_q3
     nop.i                      999
}
;;

{     .mfi
     nop.m                      999
     fma.s1    sin_poly_p3    =    sin_t,sin_poly_p1,sin_coeff_P3
     nop.i                      999
}
{     .mfi
     nop.m                      999
     fma.s1    sin_poly_p4    =    sin_rcube,sin_poly_p2,sin_r
     nop.i                      999
}
;;

{     .mfi
     nop.m                           999
     fma.s1    sin_tbl_S_tcube     =    sin_S_t,sin_tsq,f0
     nop.i                           999
}
{     .mfi
     nop.m                      999
     fma.s1    sin_poly_q12   =    sin_S_t,sin_poly_q1,sin_tbl_S
     nop.i                      999
}
;;

{     .mfi
     nop.m                 999
     fma.d.s1  sin_of_r  =    sin_r7,sin_poly_p3,sin_poly_p4
     nop.i                 999
}
;;

{     .mfi
     nop.m                           999
     fma.d.s1  sin_tbl_S_cos_of_r  =    sin_tbl_S_tcube,sin_poly_q3456,sin_poly_q12
     nop.i                           999
}
{     .mfi
     nop.m                           999
     fmpy.s0   fp_tmp = fp_tmp, fp_tmp  // Dummy mult to set inexact
     nop.i                           999
}
;;


.pred.rel "mutex",p6,p7    //SIN_Sin_Flag, SIN_Cos_Flag
{     .mfi
               nop.m            999
//(SIN_Sin_Flag) fma.s     f8   =    sin_tbl_C,sin_of_r,sin_tbl_S_cos_of_r
(p6) fma.s     f8   =    sin_tbl_C,sin_of_r,sin_tbl_S_cos_of_r
               nop.i            999
}
{     .mfb
               nop.m            999
//(SIN_Cos_Flag) fnma.s    f8   =    sin_tbl_C,sin_of_r,sin_tbl_S_cos_of_r
(p7) fnma.s    f8   =    sin_tbl_C,sin_of_r,sin_tbl_S_cos_of_r
               br.ret.sptk     b0
}

.endp sinf
ASM_SIZE_DIRECTIVE(sinf)


.proc SIN_DOUBLE 
SIN_DOUBLE:
.prologue
{ .mfi
        nop.m 0
        nop.f 0
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs
}
;;

{ .mfi
        mov GR_SAVE_GP=gp
        nop.f 0
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0
}

.body
{ .mmb
       nop.m 999
       nop.m 999
       br.call.sptk.many   b0=sin 
}
;;

{ .mfi
       mov gp        = GR_SAVE_GP
       nop.f 999
       mov b0        = GR_SAVE_B0
}
;;

{ .mfi
      nop.m 999
      fma.s f8 = f8,f1,f0
(p0)  mov ar.pfs    = GR_SAVE_PFS
}
{ .mib
      nop.m 999
      nop.i 999
(p0)  br.ret.sptk     b0 
}
;;

.endp  SIN_DOUBLE
ASM_SIZE_DIRECTIVE(SIN_DOUBLE)


.proc COS_DOUBLE 
COS_DOUBLE:
.prologue
{ .mfi
        nop.m 0
        nop.f 0
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs
}
;;

{ .mfi
        mov GR_SAVE_GP=gp
        nop.f 0
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0
}

.body
{ .mmb
       nop.m 999
       nop.m 999
       br.call.sptk.many   b0=cos 
}
;;

{ .mfi
       mov gp        = GR_SAVE_GP
       nop.f 999
       mov b0        = GR_SAVE_B0
}
;;

{ .mfi
      nop.m 999
      fma.s f8 = f8,f1,f0
(p0)  mov ar.pfs    = GR_SAVE_PFS
}
{ .mib
      nop.m 999
      nop.i 999
(p0)  br.ret.sptk     b0 
}
;;

.endp  COS_DOUBLE
ASM_SIZE_DIRECTIVE(COS_DOUBLE)



.type sin,@function
.global sin 
.type cos,@function
.global cos 
