.file "libm_atan2_reg.s"

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
// 4/04/00  Unwind support added

#include "libm_support.h"

.data

.align 64
ASM_TYPE_DIRECTIVE(Constants_atan#,@object)
Constants_atan:
data4   0x54442D18, 0x3FF921FB, 0x248D3132, 0x3E000000
// double pi/2, single lo_pi/2, two**(-3)
data4   0xAAAAAAA3, 0xAAAAAAAA, 0x0000BFFD, 0x00000000 // P_1
data4   0xCCCC54B2, 0xCCCCCCCC, 0x00003FFC, 0x00000000 // P_2
data4   0x47E4D0C2, 0x92492492, 0x0000BFFC, 0x00000000 // P_3
data4   0x58870889, 0xE38E38E0, 0x00003FFB, 0x00000000 // P_4
data4   0x290149F8, 0xBA2E895B, 0x0000BFFB, 0x00000000 // P_5
data4   0x250F733D, 0x9D88E6D4, 0x00003FFB, 0x00000000 // P_6
data4   0xFB8745A0, 0x884E51FF, 0x0000BFFB, 0x00000000 // P_7
data4   0x394396BD, 0xE1C7412B, 0x00003FFA, 0x00000000 // P_8
data4   0xAAAAA52F, 0xAAAAAAAA, 0x0000BFFD, 0x00000000 // Q_1
data4   0xC75B60D3, 0xCCCCCCCC, 0x00003FFC, 0x00000000 // Q_2
data4   0x011F1940, 0x924923AD, 0x0000BFFC, 0x00000000 // Q_3
data4   0x2A5F89BD, 0xE36F716D, 0x00003FFB, 0x00000000 // Q_4
//      Entries Tbl_hi  (double precision)
//      B = 1+Index/16+1/32  Index = 0
//      Entries Tbl_lo (single precision)
//      B = 1+Index/16+1/32  Index = 0
data4   0xA935BD8E, 0x3FE9A000, 0x23ACA08F, 0x00000000
// Entries Tbl_hi  (double precision) Index = 0,1,...,15
// B = 2^(-1)*(1+Index/16+1/32)
// Entries Tbl_lo (single precision)
// Index = 0,1,...,15  B = 2^(-1)*(1+Index/16+1/32)
data4   0x7F175A34, 0x3FDE77EB, 0x238729EE, 0x00000000
data4   0x73C1A40B, 0x3FE0039C, 0x249334DB, 0x00000000
data4   0x5B5B43DA, 0x3FE0C614, 0x22CBA7D1, 0x00000000
data4   0x88BE7C13, 0x3FE1835A, 0x246310E7, 0x00000000
data4   0xE2CC9E6A, 0x3FE23B71, 0x236210E5, 0x00000000
data4   0x8406CBCA, 0x3FE2EE62, 0x2462EAF5, 0x00000000
data4   0x1CD41719, 0x3FE39C39, 0x24B73EF3, 0x00000000
data4   0x5B795B55, 0x3FE44506, 0x24C11260, 0x00000000
data4   0x5BB6EC04, 0x3FE4E8DE, 0x242519EE, 0x00000000
data4   0x1F732FBA, 0x3FE587D8, 0x24D4346C, 0x00000000
data4   0x115D7B8D, 0x3FE6220D, 0x24ED487B, 0x00000000
data4   0x920B3D98, 0x3FE6B798, 0x2495FF1E, 0x00000000
data4   0x8FBA8E0F, 0x3FE74897, 0x223D9531, 0x00000000
data4   0x289FA093, 0x3FE7D528, 0x242B0411, 0x00000000
data4   0x576CC2C5, 0x3FE85D69, 0x2335B374, 0x00000000
data4   0xA99CC05D, 0x3FE8E17A, 0x24C27CFB, 0x00000000
//
//      Entries Tbl_hi  (double precision) Index = 0,1,...,15
//      B = 2^(-2)*(1+Index/16+1/32)
//      Entries Tbl_lo (single precision)
//      Index = 0,1,...,15  B = 2^(-2)*(1+Index/16+1/32)
//
data4   0x510665B5, 0x3FD025FA, 0x24263482, 0x00000000
data4   0x362431C9, 0x3FD1151A, 0x242C8DC9, 0x00000000
data4   0x67E47C95, 0x3FD20255, 0x245CF9BA, 0x00000000
data4   0x7A823CFE, 0x3FD2ED98, 0x235C892C, 0x00000000
data4   0x29271134, 0x3FD3D6D1, 0x2389BE52, 0x00000000
data4   0x586890E6, 0x3FD4BDEE, 0x24436471, 0x00000000
data4   0x175E0F4E, 0x3FD5A2E0, 0x2389DBD4, 0x00000000
data4   0x9F5FA6FD, 0x3FD68597, 0x2476D43F, 0x00000000
data4   0x52817501, 0x3FD76607, 0x24711774, 0x00000000
data4   0xB8DF95D7, 0x3FD84422, 0x23EBB501, 0x00000000
data4   0x7CD0C662, 0x3FD91FDE, 0x23883A0C, 0x00000000
data4   0x66168001, 0x3FD9F930, 0x240DF63F, 0x00000000
data4   0x5422058B, 0x3FDAD00F, 0x23FE261A, 0x00000000
data4   0x378624A5, 0x3FDBA473, 0x23A8CD0E, 0x00000000
data4   0x0AAD71F8, 0x3FDC7655, 0x2422D1D0, 0x00000000
data4   0xC9EC862B, 0x3FDD45AE, 0x2344A109, 0x00000000
//
//      Entries Tbl_hi  (double precision) Index = 0,1,...,15
//      B = 2^(-3)*(1+Index/16+1/32)
//      Entries Tbl_lo (single precision)
//      Index = 0,1,...,15  B = 2^(-3)*(1+Index/16+1/32)
//
data4   0x84212B3D, 0x3FC068D5, 0x239874B6, 0x00000000
data4   0x41060850, 0x3FC16465, 0x2335E774, 0x00000000
data4   0x171A535C, 0x3FC25F6E, 0x233E36BE, 0x00000000
data4   0xEDEB99A3, 0x3FC359E8, 0x239680A3, 0x00000000
data4   0xC6092A9E, 0x3FC453CE, 0x230FB29E, 0x00000000
data4   0xBA11570A, 0x3FC54D18, 0x230C1418, 0x00000000
data4   0xFFB3AA73, 0x3FC645BF, 0x23F0564A, 0x00000000
data4   0xE8A7D201, 0x3FC73DBD, 0x23D4A5E1, 0x00000000
data4   0xE398EBC7, 0x3FC8350B, 0x23D4ADDA, 0x00000000
data4   0x7D050271, 0x3FC92BA3, 0x23BCB085, 0x00000000
data4   0x601081A5, 0x3FCA217E, 0x23BC841D, 0x00000000
data4   0x574D780B, 0x3FCB1696, 0x23CF4A8E, 0x00000000
data4   0x4D768466, 0x3FCC0AE5, 0x23BECC90, 0x00000000
data4   0x4E1D5395, 0x3FCCFE65, 0x2323DCD2, 0x00000000
data4   0x864C9D9D, 0x3FCDF110, 0x23F53F3A, 0x00000000
data4   0x451D980C, 0x3FCEE2E1, 0x23CCB11F, 0x00000000
data4   0x54442D18, 0x400921FB, 0x33145C07, 0x3CA1A626 // I two doubles
data4   0x54442D18, 0x3FF921FB, 0x33145C07, 0x3C91A626 // I_by_2 two dbls
data4   0x54442D18, 0x3FE921FB, 0x33145C07, 0x3C81A626 // I_by_4 two dbls
data4   0x7F3321D2, 0x4002D97C, 0x4C9E8A0A, 0x3C9A7939 // 3I_by_4 two dbls
ASM_SIZE_DIRECTIVE(Constants_atan#)
.section .text

.proc __libm_atan2_reg#
.global __libm_atan2_reg#
.align 64
__libm_atan2_reg:


{ .mfi
       alloc  r32 = ar.pfs,0,20,4,0
(p0)   mov f32 = f8
       nop.i 0
} 
{ .mmi
      nop.m 0
(p0)  addl           r39   = @ltoff(Constants_atan#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 r39 = [r39]
      nop.m 999
      nop.i 999
}
;;

{ .mfi
       nop 999	// EMbo added ...
(p0)   mov f33 = f9
 nop.i 0
 } { .mfi
       nop 999	// EMbo added ...
(p0)   fclass.nm.unc p9,p0 = f32 ,0x1FF
       nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.nm.unc p8,p0 = f33 ,0x1FF
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc  p6,p0 = f33 ,0x103
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc  p7,p0 = f32 ,0x103
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc p12,p0 = f33 ,0x0C3
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     Check for NatVals.
//     Check for EM Unsupporteds
//     Check for NaNs.
//
(p0)   fclass.m.unc p13,p0 = f32 ,0x0C3
(p6)   br.cond.sptk L(ATAN_NATVAL);;
 } { .mbb
	nop 999	// EMbo added ...
(p7)   br.cond.sptk L(ATAN_NATVAL)
(p8)   br.cond.sptk L(ATAN_UNSUPPORTED);;
 } { .mib
(p0)   add r40 = 96, r39
	nop 999	// EMbo added ...
(p9)   br.cond.sptk L(ATAN_UNSUPPORTED);;
 } { .mib
(p0)   ldfd  f50 = [r39],8
	nop 999	// EMbo added ...
(p12)  br.cond.sptk L(ATAN_NAN);;
 } { .mfb
	nop 999	// EMbo added ...
(p0)   fnorm.s1 f33 = f33
(p13)  br.cond.sptk L(ATAN_NAN);;
 } { .mfi
(p0)   ldfs  f51 = [r39],4
//
//     Remove sign bits from exponents
//     Load 2**(-3)
//     Normalize the input argument.
//
(p0)   fnorm.s1 f32 = f32
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   mov f82 = f1
	nop 999;;	// EMbo added ...
 } { .mmi
	nop 999;;	// EMbo added ...
(p0)   ldfs  f78 = [r39],180
	nop 999;;	// EMbo added ...
 } { .mmi
(p0)   getf.exp r36 = f33;;
//
//     Get exp and sign of ArgX
//     Get exp and sign of ArgY
//     Load 2**(-3) and increment ptr to Q_4.
//
(p0)   getf.exp r37 = f32
(p0)   shr.u r36 = r36,17;;
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmerge.s f84 =  f1,f32
(p0)   shr.u r37 = r37,17;;
 } { .mfi
	nop 999	// EMbo added ...
//
//     ArgX_abs = |ArgX|
//     ArgY_abs = |ArgY|
//     sign_X is sign bit of ArgX
//     sign_Y is sign bit of ArgY
//
(p0)   fmerge.s f83 =  f1,f33
(p0)   cmp.eq.unc  p8,p9 = 0x00000, r37;;
 } { .mfi
	nop 999	// EMbo added ...
(p8)   fadd.s1 f34 = f0, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p9)   fsub.s1 f34 = f0, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmin.s1 f36 = f83, f84
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmax.s1 f35 = f83, f84
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     Is ArgX_abs >= ArgY_abs
//     Is sign_Y == 0?
//
(p0)   fcmp.ge.s1  p6,p7 = f83,f84
	nop 999;;	// EMbo added ...
 } { .mii
(p6)   cmp.eq.unc  p10, p11 =  0x00000, r36
(p6)   add r38 = r0, r0;;
//
//     U = max(ArgX_abs,ArgY_abs)
//     V = min(ArgX_abs,ArgY_abs)
//     if p6, swap = 0
//     if p7, swap = 1
//
//
//     Let M = 1.0
//     if p8, s_Y = 1.0
//     if p9, s_Y = -1.0
//
(p7)   add r38 = 1,r0;;
 } { .mfi
	nop 999	// EMbo added ...
(p0)   frcpa.s1 f37, p6 =  f36, f35
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     E = frcpa(V,U)
//
(p10)  fsub.s1 f82 = f82, f1
(p6)   br.cond.sptk L(ATAN_STEP2);;
 } { .mib
	nop 999	// EMbo added ...
	nop 999	// EMbo added ...
// /**************************************************/
// /********************* STEP2 **********************/
// /**************************************************/
(p0)   br.cond.spnt L(ATAN_SPECIAL_HANDLING);;
 }
L(ATAN_STEP2):
 { .mlx
	nop 999	// EMbo added ...
(p0)   movl r47 =  0x8400000000000000
 } { .mlx
	nop 999	// EMbo added ...
(p0)   movl r48 =  0x0000000000000100;;
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f38 = f37, f36
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fcmp.lt.unc.s0  p0,p9 = f9,f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fcmp.lt.unc.s0  p0,p8 = f8,f1
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     Q = E * V
//
(p11)  fadd.s1 f82 = f82, f1
	nop 999;;	// EMbo added ...
 } { .mfi
(p0)   getf.sig r46 = f38
(p0)   fcmp.lt.unc p6,p7 = f38,f78
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f38 = f37, f36
(p0)   extr.u r42 = r46, 59, 4;;
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f50 = f82, f50
(p0)   dep r47 = r42, r47, 59, 4
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f51 = f82, f51
	nop 999;;	// EMbo added ...
 } { .mmi
	nop 999;;	// EMbo added ...
//
//     Is Q < 2**(-3)?
//
//
//     Do fcmp to raise any denormal operand
//     exceptions.
//
(p0)   getf.exp r45 = f38
	nop 999;;	// EMbo added ...
 } { .mib
//
//     lookup = b_1 b_2 b_3 B_4
//
//
//     Generate 1.b_1 b_2 b_3 b_4 1 0 0 0 ... 0
//
(p0)   andcm r41 = 0x0003, r45
	nop 999	// EMbo added ...
//
//     We waited a few extra cycles so P_lo and P_hi could be calculated.
//     Load the constant 256 for loading up table entries.
//
//    /**************************************************/
//    /********************* STEP3 **********************/
//    /**************************************************/
(p6)   br.cond.spnt L(ATAN_POLY);;
 } { .mii
(p0)   setf.sig f39 = r47
(p0)   cmp.eq.unc  p8, p9 =  0x0000, r41
//
//     z_hi = s exp 1.b_1 b_2 b_3 b_4 1 0 0 0 ... 0
//     point to beginning of Tbl_hi entries - k = 0.
//
(p0)   add r40 = 16, r39
 } { .mmi
(p0)   ldfe f73 = [r39],-16;;
(p9)   sub r41 = r41,r0,1
(p9)   add r40 = 16,r40
 } { .mfi
(p8)   ldfd  f48 = [r40],8
(p0)   fmpy.s1 f50 = f34, f50
(p0)   xor r38 = r36,r38;;
 } { .mmi
(p0)   ldfe f71 = [r39],-16;;
(p8)   ldfs  f49 = [r40],8
(p9)   pmpy2.r r41 = r41,r48;;
 } { .mfi
(p0)   ldfe f69 = [r39],-16
//
//     Let z_hi have exponent and sign of original Q
//     Load the Tbl_hi(0) else, increment pointer.
//
(p0)   fmerge.se f39 =  f38,f39
(p9)   shladd r42 = r42,0x0004,r41;;
 } { .mmi
(p9)   add r40 = r40, r42;;
(p9)   ldfd  f48 = [r40],8
	nop 999;;	// EMbo added ...
 } { .mmi
(p0)   ldfe f67 = [r39],-16;;
(p9)   ldfs  f49 = [r40],8
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     U_prime_hi = U + V * z_hi
//     Load the Tbl_lo(0)
//
(p0)   fma.s1 f40 = f36, f39, f35
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fnma.s1 f42 = f35, f39, f36
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   mov f52 = f48
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   frcpa.s1 f43, p6 =  f1, f40
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     U_prime_lo = U - U_prime_hi
//     k = k * 256 - result can be 0, 256, or 512.
//
(p0)   fsub.s1 f41 = f35, f40
(p0)   cmp.eq.unc  p7, p6 =  0x00000, r38
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f52 = f34, f52
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p7)   fadd.s1 f54 = f0, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fsub.s1 f54 = f0, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fnma.s1 f80 = f43, f40, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fadd.s1 f79 = f41, f40
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f41 = f36, f39, f41
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f56 = f54, f52, f50
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f43 = f80, f43, f43
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     U_prime_lo =  U - U_hold
//     lookup -> lookup * 16 + k
//
//
//     V_prime =  V - U * z_hi
//     U_prime_lo =  V * z_hi + U_prime_lo
//
(p0)   fsub.s1 f79 = f35, f79
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fnma.s1 f80 = f43, f40, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     C_hi = frcpa(1,U_prime_hi)
//     U_prime_lo = U_prime_lo + U_hold
//
//
//     C_hi_hold = 1 - C_hi * U_prime_hi (1)
//
//
//     C_hi = C_hi + C_hi * C_hi_hold    (1)
//
//
//     C_hi_hold = 1 - C_hi * U_prime_hi (2)
//
(p0)   fadd.s1 f41 = f41, f79
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     C_hi = C_hi + C_hi * C_hi_hold    (2)
//
(p0)   fma.s1 f43 = f80, f43, f43
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     C_hi_hold = 1 - C_hi * U_prime_hi (3)
//
(p0)   fnma.s1 f80 = f43, f40, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     C_hi = C_hi + C_hi * C_hi_hold    (3)
//
(p0)   fma.s1 f43 = f80, f43, f43
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     w_hi = V_prime * C_hi
//
(p0)   fmpy.s1 f44 = f42, f43
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f46 = f44, f44
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     wsq = w_hi * w_hi
//     w_lo =  = V_prime - w_hi * U_prime_hi
//
(p0)   fnma.s1 f45 = f44, f40, f42
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f47 = f46, f73, f71
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     poly = Q_3 + wsq * Q_4
//     w_lo =  = w_lo - w_hi * U_prime_lo
//
(p0)   fnma.s1 f45 = f44, f41, f45
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f47 = f46, f47, f69
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     poly = Q_2 + wsq * poly
//     w_lo =  = w_lo * C_hi
//
(p0)   fmpy.s1 f45 = f43, f45
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f47 = f46, f47, f67
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     poly = Q_1 + wsq * poly
//     A_lo = Tbl_lo + w_lo
//     swap = xor(swap,sign_X)
//
(p0)   fadd.s1 f53 = f49, f45
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     Is (swap) != 0 ?
//     poly = wsq * poly
//     A_hi = Tbl_hi
//
(p0)   fmpy.s1 f47 = f46, f47
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     poly = wsq * poly
//
//
//     if (p6) sigma = -1.0
//     if (p7) sigma =  1.0
//
(p0)   fmpy.s1 f47 = f44, f47
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     P_hi = s_Y * P_hi
//     A_lo = A_lo + poly
//
(p0)   fadd.s1 f53 = f53, f47
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     A_lo = A_lo + w_hi
//     A_hi = s_Y * A_hi
//
(p0)   fadd.s1 f53 = f53, f44
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     result_hi = P_hi + sigma * A_hi
//     result_lo = P_lo + sigma * A_lo
//
(p0)   fma.s1 f55 = f54, f53, f51
(p0)   br.cond.sptk L(RETURN_ATAN);;
} 
//
//     result  =  result_hi + result_lo * s_Y  (User Supplied Rounding Mode)
//
//     (p0)   fma.d.s0 f57 = f55, f34, f56
//
// /**************************************************/
// /********************* STEP4 **********************/
// /**************************************************/
//
L(ATAN_POLY):
{ .mmi
(p0)   xor r38 = r36,r38
(p0)  addl           r39   = @ltoff(Constants_atan#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 r39 = [r39]
      nop.m 999
      nop.i 999
}
;;


{ .mlx
	nop 999	// EMbo added ...
(p0)   movl r47 =  0x24005;;
 } { .mfi
(p0)   add r39 = 128, r39
(p0)   fnma.s1 f81 = f37, f35, f1
(p0)   cmp.eq.unc  p7, p6 =  0x00000, r38;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f77 = [r39],-16
//
//     Iterate 3 times E = E + E*(1.0 - E*U)
//     Also load P_8, P_7, P_6, P_5, P_4
//     E_hold = 1.0 - E * U     (1)
//     A_temp = Q
//
(p0)   mov f85 = f38;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f76 = [r39],-16
(p6)   fsub.s1 f54 = f0, f1;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f75 = [r39],-16
//
//     E = E + E_hold*E         (1)
//     Point to P_8.
//
(p0)   fma.s1 f37 = f37, f81, f37;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f74 = [r39],-16
(p0)   fnma.s1 f64 = f85, f35, f36;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f72 = [r39],-16
(p7)   fadd.s1 f54 = f0, f1;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f70 = [r39],-16
//
//     E_hold = 1.0 - E * U     (2)
//
(p0)   fnma.s1 f81 = f37, f35, f1;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f68 = [r39],-16
(p0)   fmpy.s1 f50 = f34, f50;;
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfe f66 = [r39],-16
(p0)   fmpy.d.s0 f67 = f67, f67
 } { .mfi
	nop 999	// EMbo added ...
//
//     E = E + E_hold*E         (2)
//
(p0)   fma.s1 f37 = f37, f81, f37
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     E_hold = 1.0 - E * U     (3)
//
(p0)   fnma.s1 f81 = f37, f35, f1
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     E = E + E_hold*E         (3)
//     At this point E approximates 1/U to roughly working precision
//     z = V*E approximates V/U
//
(p0)   fma.s1 f37 = f37, f81, f37
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     z =   V * E
//
(p0)   fmpy.s1 f59 = f36, f37
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f64 = f64, f37
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     zsq = z * z
//     Also load P_3
//
(p0)   fmpy.s1 f60 = f59, f59
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fadd.s1 f52 = f85, f64
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f62 = f60, f77, f76
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f63 = f60, f70, f68
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     z8 = zsq * zsq
//     Also load P_2
//
(p0)   fmpy.s1 f61 = f60, f60
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fsub.s1 f85 = f85, f52
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmerge.s  f65 = f52,f52
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f62 = f60, f62, f75
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f63 = f60, f63, f66
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     z8 = z8 * z8
//     Also load P_1
//     poly1 = _4 + zsq*(P_5 + zsq*(P_6 + zsq*(P_7 + zsq*P_8)))
//     poly2 = zsq*(P_1 + zsq*(P_2 + zsq*P_3))
//
//
//     poly1 = P_7 + zsq * P_8
//     poly2 = P_2 + zsq * P_3
//     poly1 = P_4 + zsq*(P_5 + zsq*(P_6 + zsq*poly1))
//     poly2 = zsq*(P_1 + zsq*poly2)
//
//
//     poly1 = P_6 + zsq * poly1
//     poly2 = P_1 + zsq * poly2
//     poly1 = P_4 + zsq*(P_5 + zsq*poly1)
//     poly2 = zsq*poly2
//
(p0)   fmpy.s1 f61 = f61, f61
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fadd.s1 f64 = f85, f64
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f62 = f60, f62, f74
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     poly1 = P_5 + zsq * poly1
//     poly2 = zsq * poly2
//     poly1 = P_4 + zsq*poly1
//
(p0)   fmpy.s1 f63 = f63, f60
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     poly1 = P_4 + zsq * poly1
//     swap = xor(swap,sign_X)
//
(p0)   fma.s1 f62 = f60, f62, f72
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     poly = z8*poly1 + poly2  (Typo in writeup)
//     Is (swap) != 0 ?
//
//
//     z_lo = V - A_temp * U
//     if (p7) sigma =  1.0
//     Writeup shows A_temp as A_hi
//
//
//     z_lo = z_lo * E
//     if (p6) sigma = -1.0
//     z_lo = (V - A_temp * U) *E
//
//
//     Fixup added to force inexact later -
//     A_hi = A_temp + z_lo
//     z_lo = (A_temp - A_hi) + z_lo
//     z_lo = A_hi - z_lo -A_hi + z_lo = about 0
//
(p0)   fma.s1 f47 = f61, f62, f63
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     A_lo = z * poly + z_lo
//
(p0)   fma.s1 f53 = f59, f47, f64
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fadd.s1  f52 = f65, f53
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fsub.s1  f65 = f65, f52
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fmpy.s1 f52 = f34, f52
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fadd.s1  f53 = f65, f53
	nop 999	// EMbo added ...
 } { .mfi
(p0)   setf.exp f65 = r47
(p0)   fma.s1 f56 = f54, f52, f50
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc  p6,p0 = f53,0x007
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     P_hi = s_Y * P_hi
//     A_hi = s_Y * A_hi
//
//
//     result_hi = P_hi + sigma * A_hi
//
(p6)   mov f53 = f65
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     tmp = P_hi - result_hi
//
(p0)   fsub.s1 f65 = f50, f56
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fma.s1 f65 = f52, f54, f65
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     tmp   = sigma * A_hi  + tmp
//     sigma = A_lo * sigma  + P_lo
//
(p0)   fma.s1 f54 = f53, f54, f51
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     result_lo = s_Y * sigma + tmp
//
(p0)   fma.s1 f55 = f34, f54, f65
	nop 999;;	// EMbo added ...
 } { .mfb
       nop.m 0
       mov f34 = f1
(p0)   br.cond.sptk L(RETURN_ATAN);;
}
//
//     result  =  result_hi + result_lo  (User Supplied Rounding Mode)
//
//     (p0)   fadd.d.s0 f57 = f55, f56
L(ATAN_UNSUPPORTED):
L(ATAN_NATVAL):
 { .mfb
	nop 999	// EMbo added ...
//
//     Deal with the NatVal and unsupported cases.
//     Raise invalid if warrented.
//
(p0)   fmpy.d.s0 f57 = f8, f9
br.cond.sptk L(RETURN_ATAN);;
 }
L(ATAN_NAN):
 { .mfb
	nop 999	// EMbo added ...
//
//     If only one NaN, then generate the resulting
//     NaN and return - may raise invalid.
//
(p0)   fmpy.d.s0 f57 = f8, f9
(p0)   br.cond.sptk L(RETURN_ATAN);;
 }
L(ATAN_SPECIAL_HANDLING):

 { .mmf
(p0)   addl           r39   = @ltoff(Constants_atan#), gp
       nop.m 999
(p0)   fcmp.lt.s0 p0,p7 = f8,f1
 } 
;;

//
//     Raise denormal operand faults if necessary
//

{ .mfi
       ld8 r39 = [r39]
(p0)   fcmp.lt.s0 p0,p6 = f9,f1
	nop 999;;	// EMbo added ...
} 
;;



{ .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc p6,p7 = f32,0x007
	nop 999;;	// EMbo added ...
 } { .mlx
	nop 999	// EMbo added ...
(p0)   movl r47 = 992;;
 } { .mib
(p0)   add r39 = r39, r47
	nop 999	// EMbo added ...
(p7)   br.cond.sptk L(ATAN_ArgY_Not_ZERO);;
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fclass.m.unc p14,p0 = f33,0x035
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fclass.m.unc p15,p0 = f33,0x036
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fclass.m.unc p13,p0 = f33,0x007
	nop 999	// EMbo added ...
 } { .mfi
(p0)   ldfd  f56 = [r39],8
	nop 999	// EMbo added ...
	nop 999;;	// EMbo added ...
 } { .mfi
(p0)   ldfd  f55 = [r39],-8
(p14)  fmerge.s f56 =  f32,f0
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     Return sign_Y * 0 when Y = +/-0 and X > 0
//
(p14)  fmerge.s f55 =  f32,f0
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p15)  fmerge.s f56 =  f32,f56
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     Return sign_Y * PI when X <  -0
//
//
(p15)  fmerge.s f55 =  f32,f55
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fadd.d.s0 f57 = f56,f55
      nop.i 0
 } { .bbb
//
//     Call error support function for atan(0,0)
//     - expected value already computed.
//
    nop.b 0
    nop.b 0
(p0)  br.cond.sptk L(RETURN_ATAN)
 }
L(ATAN_ArgY_Not_ZERO):
 { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc p9,p10 = f32,0x023
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
(p9)   fclass.m.unc p6,p0 = f33,0x017
(p10)  br.cond.sptk L(ATAN_ArgY_Not_INF);;
 } { .mfi
(p6)   add r39 = 16,r39
(p9)   fclass.m.unc p7,p0 = f33,0x021
	nop 999;;	// EMbo added ...
 } { .mmf
	nop 999	// EMbo added ...
(p0)   ldfd  f56 = [r39],8
(p9)   fclass.m.unc p8,p0 = f33,0x022;;
 } { .mbb
(p0)   ldfd  f55 = [r39],-8
	nop 999	// EMbo added ...
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fmerge.s f56 =  f32,f56
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fmerge.s f55 =  f32,f55
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     Load I/2 and adjust its sign.
//     Return +I/2 when ArgY = +Inf and ArgX = +/-0,normal
//     Return -I/2 when ArgY = -Inf and ArgX = +/-0,normal
//
(p6)   fadd.d.s0  f57 =   f56,  f55
(p6)   br.cond.sptk L(RETURN_ATAN);;
 } { .mmi
(p7)   add r39 = 32,r39;;
(p7)   ldfd  f56 = [r39],8
	nop 999;;	// EMbo added ...
 } { .mmi
	nop 999;;	// EMbo added ...
(p7)   ldfd  f55 = [r39],-8
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p7)   fmerge.s f56 =  f32,f56
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p7)   fmerge.s f55 =  f32,f55
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     Load PI/4 and adjust its sign.
//     Return +PI/4 when ArgY = +Inf and ArgX = +Inf
//     Return -PI/4 when ArgY = -Inf and ArgX = +Inf
//
(p7)   fadd.d.s0  f57 =   f56,  f55
(p7)   br.cond.sptk L(RETURN_ATAN);;
 } { .mmi
(p8)   add r39 = 48,r39;;
(p8)   ldfd f56 =[r39],8
	nop 999;;	// EMbo added ...
 } { .mmi
	nop 999;;	// EMbo added ...
(p8)   ldfd f55 =[r39],-8
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p8)   fmerge.s f56 =  f32,f56
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p8)   fmerge.s f55 =  f32,f55
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     Load I/4 and adjust its sign.
//     Return +3I/4 when ArgY = +Inf and ArgX = -Inf
//     Return -3I/4 when ArgY = -Inf and ArgX = -Inf
//
(p8)   fadd.d.s0  f57 =   f56,  f55
(p8)   br.cond.sptk L(RETURN_ATAN);;
 }
L(ATAN_ArgY_Not_INF):
 { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc  p6,p0 = f33,0x007
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc  p7,p0 = f33,0x021
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p0)   fclass.m.unc  p8,p0 = f33,0x022
(p6)   add r39 = 16,r39;;
 } { .mfi
(p6)   ldfd f56 =[r39],8
	nop 999	// EMbo added ...
	nop 999;;	// EMbo added ...
 } { .mmi
	nop 999;;	// EMbo added ...
(p6)   ldfd f55 =[r39],-8
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fmerge.s f56 =  f32,f56
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p6)   fmerge.s f55 =  f32,f55
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     return = sign_Y * I/2 when ArgX = +/-0
//
(p6)   fadd.d.s0 f57 = f56, f55
(p6)   br.cond.sptk L(RETURN_ATAN);;
 } { .mfi
	nop 999	// EMbo added ...
(p7)   fmerge.s f56 =  f32,f0
	nop 999	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p7)   fmerge.s f55 =  f32,f0
	nop 999;;	// EMbo added ...
 } { .mfb
	nop 999	// EMbo added ...
//
//     return = sign_Y * 0 when ArgX = Inf
//
(p7)   fadd.d.s0 f57 = f56, f55
(p7)   br.cond.sptk L(RETURN_ATAN);;
 } { .mfi
(p8)   ldfd f56 = [r39],8
	nop 999	// EMbo added ...
	nop 999;;	// EMbo added ...
 } { .mmi
	nop 999;;	// EMbo added ...
(p8)   ldfd f55 = [r39],-8
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p8)   fmerge.s f56 =  f32,f56
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
(p8)   fmerge.s f55 =  f32,f55
	nop 999;;	// EMbo added ...
 } { .mfi
	nop 999	// EMbo added ...
//
//     return = sign_Y * I when ArgX = -Inf
//
(p8)   fadd.d.s0  f57 =   f56,  f55
	nop 999	// EMbo added ...
 };;
L(RETURN_ATAN):
// mov    f8     = f57 ;;
// The answer is in f57.
// But Z_hi is f56
//     Z_lo is f55
//     s_Y  is f34
//     W is in f9 and untouched

{ .mfi
	nop 999	
mov    f8     = f56
        nop.i 0
};;

{ .mfi
	nop 999	
mov    f10    = f55
        nop.i 999
}
{ .mfb
	nop 999	
mov    f11    = f34
br.ret.sptk   b0
};;

.endp __libm_atan2_reg
ASM_SIZE_DIRECTIVE(__libm_atan2_reg)
