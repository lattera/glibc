.file "sinhf.s"

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
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 10/12/00 Update to set denormal operand and underflow flags
// 1/22/01  Fixed to set inexact flag for small args.
//
// API
//==============================================================
// float = sinhf(float)
// input  floating point f8
// output floating point f8
//
// Registers used
//==============================================================
// general registers: 
// r32 -> r47
// predicate registers used:
// p6 p7 p8 p9
// floating-point registers used:
// f9 -> f15; f32 -> f45; 
// f8 has input, then output
//
// Overview of operation
//==============================================================
// There are four paths
// 1. |x| < 0.25        SINH_BY_POLY
// 2. |x| < 32          SINH_BY_TBL
// 3. |x| < 2^14        SINH_BY_EXP
// 4. |x_ >= 2^14       SINH_HUGE
//
// For double extended we get infinity for x >= 400c b174 ddc0 31ae c0ea
//                                           >= 1.0110001.... x 2^13
//                                           >= 11357.2166
//
// But for double we get infinity for x >= 408633ce8fb9f87e
//                                      >= 1.0110...x 2^9
//                                      >= +7.10476e+002
//
// And for single we get infinity for x >= 42b3a496
//                                      >= 1.0110... 2^6
//                                      >= 89.8215
//
// SAFE: If there is danger of overflow set SAFE to 0
//       NOT implemented: if there is danger of underflow, set SAFE to 0
// SAFE for all paths listed below
//
// 1. SINH_BY_POLY
// ===============
// If |x| is less than the tiny threshold, then clear SAFE 
// For double, the tiny threshold is -1022 = -0x3fe => -3fe + ffff = fc01
//             register-biased, this is fc01
// For single, the tiny threshold is -126  = -7e    => -7e  + ffff = ff81
// If |x| < tiny threshold, set SAFE = 0
//
// 2. SINH_BY_TBL
// =============
// SAFE: SAFE is always 1 for TBL; 
//
// 3. SINH_BY_EXP
// ==============
// There is a danger of double-extended overflow   if N-1 > 16382 = 0x3ffe
// r34 has N-1; 16382 is in register biased form, 0x13ffd
// There is danger of double overflow if N-1 > 0x3fe
//                       in register biased form, 0x103fd
// Analagously, there is danger of single overflow if N-1 > 0x7e
//                       in register biased form, 0x1007d
// SAFE: If there is danger of overflow set SAFE to 0
//
// 4. SINH_HUGE
// ============
// SAFE: SAFE is always 0 for HUGE
//

#include "libm_support.h"

// Assembly macros
//==============================================================
sinh_FR_X            = f44
sinh_FR_X2           = f9
sinh_FR_X4           = f10
sinh_FR_SGNX         = f40
sinh_FR_all_ones     = f45
sinh_FR_tmp          = f42

sinh_FR_Inv_log2by64 = f9
sinh_FR_log2by64_lo  = f11
sinh_FR_log2by64_hi  = f10

sinh_FR_A1           = f9
sinh_FR_A2           = f10
sinh_FR_A3           = f11

sinh_FR_Rcub         = f12
sinh_FR_M_temp       = f13
sinh_FR_R_temp       = f13
sinh_FR_Rsq          = f13
sinh_FR_R            = f14

sinh_FR_M            = f38

sinh_FR_B1           = f15
sinh_FR_B2           = f32
sinh_FR_B3           = f33

sinh_FR_peven_temp1  = f34
sinh_FR_peven_temp2  = f35
sinh_FR_peven        = f36

sinh_FR_podd_temp1   = f34
sinh_FR_podd_temp2   = f35
sinh_FR_podd         = f37

sinh_FR_poly_podd_temp1    =  f11 
sinh_FR_poly_podd_temp2    =  f13
sinh_FR_poly_peven_temp1   =  f11
sinh_FR_poly_peven_temp2   =  f13

sinh_FR_J_temp       = f9
sinh_FR_J            = f10

sinh_FR_Mmj          = f39

sinh_FR_N_temp1      = f11
sinh_FR_N_temp2      = f12
sinh_FR_N            = f13

sinh_FR_spos         = f14
sinh_FR_sneg         = f15

sinh_FR_Tjhi         = f32
sinh_FR_Tjlo         = f33
sinh_FR_Tmjhi        = f34
sinh_FR_Tmjlo        = f35

sinh_GR_mJ           = r35
sinh_GR_J            = r36

sinh_AD_mJ           = r38
sinh_AD_J            = r39
sinh_GR_all_ones     = r40

sinh_FR_S_hi         = f9
sinh_FR_S_hi_temp    = f10
sinh_FR_S_lo_temp1   = f11 
sinh_FR_S_lo_temp2   = f12 
sinh_FR_S_lo_temp3   = f13 

sinh_FR_S_lo         = f38
sinh_FR_C_hi         = f39

sinh_FR_C_hi_temp1   = f10
sinh_FR_Y_hi         = f11 
sinh_FR_Y_lo_temp    = f12 
sinh_FR_Y_lo         = f13 
sinh_FR_SINH         = f9

sinh_FR_P1           = f14
sinh_FR_P2           = f15
sinh_FR_P3           = f32
sinh_FR_P4           = f33
sinh_FR_P5           = f34
sinh_FR_P6           = f35

sinh_FR_TINY_THRESH  = f9

sinh_FR_SINH_temp    = f10
sinh_FR_SCALE        = f11 

sinh_FR_signed_hi_lo = f10


GR_SAVE_PFS          = r41
GR_SAVE_B0           = r42
GR_SAVE_GP           = r43

GR_Parameter_X       = r44
GR_Parameter_Y       = r45
GR_Parameter_RESULT  = r46

// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16
double_sinh_arg_reduction:
ASM_TYPE_DIRECTIVE(double_sinh_arg_reduction,@object)
   data8 0xB8AA3B295C17F0BC, 0x00004005
   data8 0xB17217F7D1000000, 0x00003FF8
   data8 0xCF79ABC9E3B39804, 0x00003FD0
ASM_SIZE_DIRECTIVE(double_sinh_arg_reduction)

double_sinh_p_table:
ASM_TYPE_DIRECTIVE(double_sinh_p_table,@object)
   data8 0xAAAAAAAAAAAAAAAB, 0x00003FFC
   data8 0x8888888888888412, 0x00003FF8
   data8 0xD00D00D00D4D39F2, 0x00003FF2
   data8 0xB8EF1D28926D8891, 0x00003FEC
   data8 0xD732377688025BE9, 0x00003FE5
   data8 0xB08AF9AE78C1239F, 0x00003FDE
ASM_SIZE_DIRECTIVE(double_sinh_p_table)

double_sinh_ab_table:
ASM_TYPE_DIRECTIVE(double_sinh_ab_table,@object)
   data8 0xAAAAAAAAAAAAAAAC, 0x00003FFC
   data8 0x88888888884ECDD5, 0x00003FF8
   data8 0xD00D0C6DCC26A86B, 0x00003FF2
   data8 0x8000000000000002, 0x00003FFE
   data8 0xAAAAAAAAAA402C77, 0x00003FFA
   data8 0xB60B6CC96BDB144D, 0x00003FF5
ASM_SIZE_DIRECTIVE(double_sinh_ab_table)

double_sinh_j_table:
ASM_TYPE_DIRECTIVE(double_sinh_j_table,@object)
   data8 0xB504F333F9DE6484, 0x00003FFE, 0x1EB2FB13, 0x00000000
   data8 0xB6FD91E328D17791, 0x00003FFE, 0x1CE2CBE2, 0x00000000
   data8 0xB8FBAF4762FB9EE9, 0x00003FFE, 0x1DDC3CBC, 0x00000000
   data8 0xBAFF5AB2133E45FB, 0x00003FFE, 0x1EE9AA34, 0x00000000
   data8 0xBD08A39F580C36BF, 0x00003FFE, 0x9EAEFDC1, 0x00000000
   data8 0xBF1799B67A731083, 0x00003FFE, 0x9DBF517B, 0x00000000
   data8 0xC12C4CCA66709456, 0x00003FFE, 0x1EF88AFB, 0x00000000
   data8 0xC346CCDA24976407, 0x00003FFE, 0x1E03B216, 0x00000000
   data8 0xC5672A115506DADD, 0x00003FFE, 0x1E78AB43, 0x00000000
   data8 0xC78D74C8ABB9B15D, 0x00003FFE, 0x9E7B1747, 0x00000000
   data8 0xC9B9BD866E2F27A3, 0x00003FFE, 0x9EFE3C0E, 0x00000000
   data8 0xCBEC14FEF2727C5D, 0x00003FFE, 0x9D36F837, 0x00000000
   data8 0xCE248C151F8480E4, 0x00003FFE, 0x9DEE53E4, 0x00000000
   data8 0xD06333DAEF2B2595, 0x00003FFE, 0x9E24AE8E, 0x00000000
   data8 0xD2A81D91F12AE45A, 0x00003FFE, 0x1D912473, 0x00000000
   data8 0xD4F35AABCFEDFA1F, 0x00003FFE, 0x1EB243BE, 0x00000000
   data8 0xD744FCCAD69D6AF4, 0x00003FFE, 0x1E669A2F, 0x00000000
   data8 0xD99D15C278AFD7B6, 0x00003FFE, 0x9BBC610A, 0x00000000
   data8 0xDBFBB797DAF23755, 0x00003FFE, 0x1E761035, 0x00000000
   data8 0xDE60F4825E0E9124, 0x00003FFE, 0x9E0BE175, 0x00000000
   data8 0xE0CCDEEC2A94E111, 0x00003FFE, 0x1CCB12A1, 0x00000000
   data8 0xE33F8972BE8A5A51, 0x00003FFE, 0x1D1BFE90, 0x00000000
   data8 0xE5B906E77C8348A8, 0x00003FFE, 0x1DF2F47A, 0x00000000
   data8 0xE8396A503C4BDC68, 0x00003FFE, 0x1EF22F22, 0x00000000
   data8 0xEAC0C6E7DD24392F, 0x00003FFE, 0x9E3F4A29, 0x00000000
   data8 0xED4F301ED9942B84, 0x00003FFE, 0x1EC01A5B, 0x00000000
   data8 0xEFE4B99BDCDAF5CB, 0x00003FFE, 0x1E8CAC3A, 0x00000000
   data8 0xF281773C59FFB13A, 0x00003FFE, 0x9DBB3FAB, 0x00000000
   data8 0xF5257D152486CC2C, 0x00003FFE, 0x1EF73A19, 0x00000000
   data8 0xF7D0DF730AD13BB9, 0x00003FFE, 0x9BB795B5, 0x00000000
   data8 0xFA83B2DB722A033A, 0x00003FFE, 0x1EF84B76, 0x00000000
   data8 0xFD3E0C0CF486C175, 0x00003FFE, 0x9EF5818B, 0x00000000
   data8 0x8000000000000000, 0x00003FFF, 0x00000000, 0x00000000
   data8 0x8164D1F3BC030773, 0x00003FFF, 0x1F77CACA, 0x00000000
   data8 0x82CD8698AC2BA1D7, 0x00003FFF, 0x1EF8A91D, 0x00000000
   data8 0x843A28C3ACDE4046, 0x00003FFF, 0x1E57C976, 0x00000000
   data8 0x85AAC367CC487B15, 0x00003FFF, 0x9EE8DA92, 0x00000000
   data8 0x871F61969E8D1010, 0x00003FFF, 0x1EE85C9F, 0x00000000
   data8 0x88980E8092DA8527, 0x00003FFF, 0x1F3BF1AF, 0x00000000
   data8 0x8A14D575496EFD9A, 0x00003FFF, 0x1D80CA1E, 0x00000000
   data8 0x8B95C1E3EA8BD6E7, 0x00003FFF, 0x9D0373AF, 0x00000000
   data8 0x8D1ADF5B7E5BA9E6, 0x00003FFF, 0x9F167097, 0x00000000
   data8 0x8EA4398B45CD53C0, 0x00003FFF, 0x1EB70051, 0x00000000
   data8 0x9031DC431466B1DC, 0x00003FFF, 0x1F6EB029, 0x00000000
   data8 0x91C3D373AB11C336, 0x00003FFF, 0x1DFD6D8E, 0x00000000
   data8 0x935A2B2F13E6E92C, 0x00003FFF, 0x9EB319B0, 0x00000000
   data8 0x94F4EFA8FEF70961, 0x00003FFF, 0x1EBA2BEB, 0x00000000
   data8 0x96942D3720185A00, 0x00003FFF, 0x1F11D537, 0x00000000
   data8 0x9837F0518DB8A96F, 0x00003FFF, 0x1F0D5A46, 0x00000000
   data8 0x99E0459320B7FA65, 0x00003FFF, 0x9E5E7BCA, 0x00000000
   data8 0x9B8D39B9D54E5539, 0x00003FFF, 0x9F3AAFD1, 0x00000000
   data8 0x9D3ED9A72CFFB751, 0x00003FFF, 0x9E86DACC, 0x00000000
   data8 0x9EF5326091A111AE, 0x00003FFF, 0x9F3EDDC2, 0x00000000
   data8 0xA0B0510FB9714FC2, 0x00003FFF, 0x1E496E3D, 0x00000000
   data8 0xA27043030C496819, 0x00003FFF, 0x9F490BF6, 0x00000000
   data8 0xA43515AE09E6809E, 0x00003FFF, 0x1DD1DB48, 0x00000000
   data8 0xA5FED6A9B15138EA, 0x00003FFF, 0x1E65EBFB, 0x00000000
   data8 0xA7CD93B4E965356A, 0x00003FFF, 0x9F427496, 0x00000000
   data8 0xA9A15AB4EA7C0EF8, 0x00003FFF, 0x1F283C4A, 0x00000000
   data8 0xAB7A39B5A93ED337, 0x00003FFF, 0x1F4B0047, 0x00000000
   data8 0xAD583EEA42A14AC6, 0x00003FFF, 0x1F130152, 0x00000000
   data8 0xAF3B78AD690A4375, 0x00003FFF, 0x9E8367C0, 0x00000000
   data8 0xB123F581D2AC2590, 0x00003FFF, 0x9F705F90, 0x00000000
   data8 0xB311C412A9112489, 0x00003FFF, 0x1EFB3C53, 0x00000000
   data8 0xB504F333F9DE6484, 0x00003FFF, 0x1F32FB13, 0x00000000
ASM_SIZE_DIRECTIVE(double_sinh_j_table)

.align 32
.global sinhf#

.section .text
.proc  sinhf#
.align 32

sinhf: 
#ifdef _LIBC
.global __ieee754_sinhf
.type __ieee754_sinhf,@function
__ieee754_sinhf:
#endif

// X infinity or NAN?
// Take invalid fault if enabled


{ .mfi
      alloc r32 = ar.pfs,0,12,4,0                  
(p0)     fclass.m.unc  p6,p0 = f8, 0xe3	//@qnan | @snan | @inf 
         mov sinh_GR_all_ones = -1
}
;;


{ .mfb
         nop.m 999
(p6)     fma.s.s0   f8 = f8,f1,f8               
(p6)     br.ret.spnt     b0 ;;                          
}

// Put 0.25 in f9; p6 true if x < 0.25
// Make constant that will generate inexact when squared
{ .mlx
         setf.sig sinh_FR_all_ones = sinh_GR_all_ones 
(p0)     movl            r32 = 0x000000000000fffd ;;         
}

{ .mfi
(p0)     setf.exp        f9 = r32                         
(p0)     fclass.m.unc  p7,p0 = f8, 0x07	//@zero
         nop.i 999 ;;
}

{ .mfb
         nop.m 999
(p0)     fmerge.s      sinh_FR_X    = f0,f8             
(p7)     br.ret.spnt     b0 ;;                          
}

// Identify denormal operands.
{ .mfi
         nop.m 999
         fclass.m.unc  p10,p0 = f8, 0x09        //  + denorm
         nop.i 999
};;
{ .mfi
         nop.m 999
         fclass.m.unc  p11,p0 = f8, 0x0a        //  - denorm
         nop.i 999 
}

{ .mfi
         nop.m 999
(p0)     fmerge.s      sinh_FR_SGNX = f8,f1             
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fcmp.lt.unc.s1  p0,p7 = sinh_FR_X,f9             
         nop.i 999 ;;
}

{ .mib
         nop.m 999
         nop.i 999
(p7)     br.cond.sptk    L(SINH_BY_TBL) ;;                      
}


L(SINH_BY_POLY): 

// POLY cannot overflow so there is no need to call __libm_error_support
// Set tiny_SAFE (p7) to 1(0) if answer is not tiny 
// Currently we do not use tiny_SAFE. So the setting of tiny_SAFE is
// commented out.
//(p0)     movl            r32            = 0x000000000000fc01           
//(p0)     setf.exp        f10            = r32                         
//(p0)     fcmp.lt.unc.s1  p6,p7          = f8,f10                     
// Here is essentially the algorithm for SINH_BY_POLY. Care is take for the order 
// of multiplication; and P_1 is not exactly 1/3!, P_2 is not exactly 1/5!, etc.
// Note that ax = |x|
// sinh(x) = sign * (series(e^x) - series(e^-x))/2
//         = sign * (ax + ax^3/3! + ax^5/5! + ax^7/7! + ax^9/9! + ax^11/11! + ax^13/13!)
//         = sign * (ax   + ax * ( ax^2 * (1/3! + ax^4 * (1/7! + ax^4*1/11!)) )
//                        + ax * ( ax^4 * (1/5! + ax^4 * (1/9! + ax^4*1/13!)) ) )
//         = sign * (ax   + ax*p_odd + (ax*p_even))
//         = sign * (ax   + Y_lo)
// sinh(x) = sign * (Y_hi + Y_lo)
// Get the values of P_x from the table
{ .mfb
(p0)  addl           r34   = @ltoff(double_sinh_p_table), gp
(p10) fma.s.s0       f8 =  f8,f8,f8
(p10) br.ret.spnt    b0
}
;;

{ .mfb
      ld8 r34 = [r34]
(p11) fnma.s.s0      f8 =  f8,f8,f8
(p11) br.ret.spnt    b0
}
;;

// Calculate sinh_FR_X2 = ax*ax and sinh_FR_X4 = ax*ax*ax*ax
{ .mmf
         nop.m 999
(p0)     ldfe            sinh_FR_P1 = [r34],16                 
(p0)     fma.s1        sinh_FR_X2 = sinh_FR_X, sinh_FR_X, f0 ;;           
}

{ .mmi
(p0)     ldfe            sinh_FR_P2 = [r34],16 ;;                 
(p0)     ldfe            sinh_FR_P3 = [r34],16                 
         nop.i 999 ;;
}

{ .mmi
(p0)     ldfe            sinh_FR_P4 = [r34],16 ;;                 
(p0)     ldfe            sinh_FR_P5 = [r34],16                 
         nop.i 999 ;;
}

{ .mfi
(p0)     ldfe            sinh_FR_P6 = [r34],16                 
(p0)     fma.s1        sinh_FR_X4 = sinh_FR_X2, sinh_FR_X2, f0         
         nop.i 999 ;;
}

// Calculate sinh_FR_podd = p_odd and sinh_FR_peven = p_even 
{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_poly_podd_temp1 = sinh_FR_X4, sinh_FR_P5, sinh_FR_P3                
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_poly_podd_temp2 = sinh_FR_X4, sinh_FR_poly_podd_temp1, sinh_FR_P1   
         nop.i 999
}

{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_poly_peven_temp1 = sinh_FR_X4, sinh_FR_P6, sinh_FR_P4               
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_podd       = sinh_FR_X2, sinh_FR_poly_podd_temp2, f0           
         nop.i 999
}

{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_poly_peven_temp2 = sinh_FR_X4, sinh_FR_poly_peven_temp1, sinh_FR_P2 
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_peven       = sinh_FR_X4, sinh_FR_poly_peven_temp2, f0         
         nop.i 999 ;;
}

// Calculate sinh_FR_Y_lo = ax*p_odd + (ax*p_even)
{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_Y_lo_temp    = sinh_FR_X, sinh_FR_peven, f0                    
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_Y_lo         = sinh_FR_X, sinh_FR_podd,  sinh_FR_Y_lo_temp          
         nop.i 999 ;;
}

// Calculate sinh_FR_SINH = Y_hi + Y_lo. Note that ax = Y_hi
{ .mfi
         nop.m 999
(p0)     fma.s1      sinh_FR_SINH        = sinh_FR_X, f1, sinh_FR_Y_lo                      
         nop.i 999 ;;
}
// Dummy multiply to generate inexact
{ .mfi
         nop.m 999
(p0)     fmpy.s0      sinh_FR_tmp = sinh_FR_all_ones, sinh_FR_all_ones
         nop.i 999
}

// Calculate f8 = sign * (Y_hi + Y_lo)
// Go to return
{ .mfb
         nop.m 999
(p0)     fma.s.s0        f8 = sinh_FR_SGNX,sinh_FR_SINH,f0                       
(p0)     br.ret.sptk     b0 ;;                          
}


L(SINH_BY_TBL): 

// Now that we are at TBL; so far all we know is that |x| >= 0.25.
// The first two steps are the same for TBL and EXP, but if we are HUGE
// we want to leave now. 
// Double-extended:
// Go to HUGE if |x| >= 2^14, 1000d (register-biased) is e = 14 (true)
// Double
// Go to HUGE if |x| >= 2^10, 10009 (register-biased) is e = 10 (true)
// Single
// Go to HUGE if |x| >= 2^7,  10006 (register-biased) is e =  7 (true)

{ .mlx
         nop.m 999
(p0)     movl            r32 = 0x0000000000010006 ;;         
}

{ .mfi
(p0)     setf.exp        f9 = r32                         
         nop.f 999
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fcmp.ge.unc.s1  p6,p7 = sinh_FR_X,f9             
         nop.i 999 ;;
}

{ .mib
         nop.m 999
         nop.i 999
(p6)     br.cond.spnt    L(SINH_HUGE) ;;                        
}

// r32 = 1
// r34 = N-1 
// r35 = N
// r36 = j
// r37 = N+1

// TBL can never overflow
// sinh(x) = sinh(B+R)
//         = sinh(B)cosh(R) + cosh(B)sinh(R)
// 
// ax = |x| = M*log2/64 + R
// B = M*log2/64
// M = 64*N + j 
//   We will calcualte M and get N as (M-j)/64
//   The division is a shift.
// exp(B)  = exp(N*log2 + j*log2/64)
//         = 2^N * 2^(j*log2/64)
// sinh(B) = 1/2(e^B -e^-B)
//         = 1/2(2^N * 2^(j*log2/64) - 2^-N * 2^(-j*log2/64)) 
// sinh(B) = (2^(N-1) * 2^(j*log2/64) - 2^(-N-1) * 2^(-j*log2/64)) 
// cosh(B) = (2^(N-1) * 2^(j*log2/64) + 2^(-N-1) * 2^(-j*log2/64)) 
// 2^(j*log2/64) is stored as Tjhi + Tjlo , j= -32,....,32
// Tjhi is double-extended (80-bit) and Tjlo is single(32-bit)
// R = ax - M*log2/64
// R = ax - M*log2_by_64_hi - M*log2_by_64_lo
// exp(R) = 1 + R +R^2(1/2! + R(1/3! + R(1/4! + ... + R(1/n!)...)
//        = 1 + p_odd + p_even
//        where the p_even uses the A coefficients and the p_even uses the B coefficients
// So sinh(R) = 1 + p_odd + p_even -(1 -p_odd -p_even)/2 = p_odd
//    cosh(R) = 1 + p_even
//    sinh(B) = S_hi + S_lo
//    cosh(B) = C_hi
// sinh(x) = sinh(B)cosh(R) + cosh(B)sinh(R)
// ******************************************************
// STEP 1 (TBL and EXP)
// ******************************************************
// Get the following constants. 
// f9  = Inv_log2by64
// f10 = log2by64_hi
// f11 = log2by64_lo

{ .mmi
(p0)  adds                 r32 = 0x1,r0      
(p0)  addl           r34   = @ltoff(double_sinh_arg_reduction), gp
      nop.i 999
}
;;

{ .mmi
      ld8 r34 = [r34]
      nop.m 999
      nop.i 999
}
;;


// We want 2^(N-1) and 2^(-N-1). So bias N-1 and -N-1 and
// put them in an exponent.
// sinh_FR_spos = 2^(N-1) and sinh_FR_sneg = 2^(-N-1)
// r39 = 0xffff + (N-1)  = 0xffff +N -1
// r40 = 0xffff - (N +1) = 0xffff -N -1

{ .mlx
         nop.m 999
(p0)     movl                r38 = 0x000000000000fffe ;; 
}

{ .mmi
(p0)     ldfe            sinh_FR_Inv_log2by64 = [r34],16 ;;       
(p0)     ldfe            sinh_FR_log2by64_hi  = [r34],16       
         nop.i 999 ;;
}

{ .mbb
(p0)     ldfe            sinh_FR_log2by64_lo  = [r34],16       
         nop.b 999
         nop.b 999 ;;
}

// Get the A coefficients
// f9  = A_1
// f10 = A_2
// f11 = A_3

{ .mmi
      nop.m 999
(p0)  addl           r34   = @ltoff(double_sinh_ab_table), gp
      nop.i 999
}
;;

{ .mmi
      ld8 r34 = [r34]
      nop.m 999
      nop.i 999
}
;;


// Calculate M and keep it as integer and floating point.
// f38 = M = round-to-integer(x*Inv_log2by64)
// sinh_FR_M = M = truncate(ax/(log2/64))
// Put the significand of M in r35
//    and the floating point representation of M in sinh_FR_M

{ .mfi
         nop.m 999
(p0)     fma.s1          sinh_FR_M      = sinh_FR_X, sinh_FR_Inv_log2by64, f0 
         nop.i 999
}

{ .mfi
(p0)     ldfe            sinh_FR_A1 = [r34],16            
         nop.f 999
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fcvt.fx.s1      sinh_FR_M_temp = sinh_FR_M                      
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fnorm.s1        sinh_FR_M      = sinh_FR_M_temp                 
         nop.i 999 ;;
}

{ .mfi
(p0)     getf.sig        r35       = sinh_FR_M_temp                 
         nop.f 999
         nop.i 999 ;;
}

// M is still in r35. Calculate j. j is the signed extension of the six lsb of M. It 
// has a range of -32 thru 31.
// r35 = M
// r36 = j 

{ .mii
         nop.m 999
         nop.i 999 ;;
(p0)     and            r36 = 0x3f, r35 ;;   
}

// Calculate R
// f13 = f44 - f12*f10 = ax - M*log2by64_hi
// f14 = f13 - f8*f11  = R = (ax - M*log2by64_hi) - M*log2by64_lo

{ .mfi
         nop.m 999
(p0)     fnma.s1           sinh_FR_R_temp = sinh_FR_M, sinh_FR_log2by64_hi, sinh_FR_X      
         nop.i 999
}

{ .mfi
(p0)     ldfe            sinh_FR_A2 = [r34],16            
         nop.f 999
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fnma.s1           sinh_FR_R      = sinh_FR_M, sinh_FR_log2by64_lo, sinh_FR_R_temp 
         nop.i 999
}

// Get the B coefficients
// f15 = B_1
// f32 = B_2
// f33 = B_3

{ .mmi
(p0)     ldfe            sinh_FR_A3 = [r34],16 ;;            
(p0)     ldfe            sinh_FR_B1 = [r34],16            
         nop.i 999 ;;
}

{ .mmi
(p0)     ldfe            sinh_FR_B2 = [r34],16 ;;            
(p0)     ldfe            sinh_FR_B3 = [r34],16            
         nop.i 999 ;;
}

{ .mii
         nop.m 999
(p0)     shl            r34 = r36,  0x2 ;;   
(p0)     sxt1           r37 = r34 ;;         
}

// ******************************************************
// STEP 2 (TBL and EXP)
// ******************************************************
// Calculate Rsquared and Rcubed in preparation for p_even and p_odd
// f12 = R*R*R
// f13 = R*R
// f14 = R <== from above

{ .mfi
         nop.m 999
(p0)     fma.s1             sinh_FR_Rsq  = sinh_FR_R,   sinh_FR_R, f0  
(p0)     shr            r36 = r37,  0x2 ;;   
}

// r34 = M-j = r35 - r36
// r35 = N = (M-j)/64

{ .mii
(p0)     sub                  r34 = r35, r36    
         nop.i 999 ;;
(p0)     shr                  r35 = r34, 0x6 ;;    
}

{ .mii
(p0)     sub                 r40 = r38, r35           
(p0)     adds                 r37 = 0x1, r35    
(p0)     add                 r39 = r38, r35 ;;           
}

// Get the address of the J table, add the offset, 
// addresses are sinh_AD_mJ and sinh_AD_J, get the T value
// f32 = T(j)_hi
// f33 = T(j)_lo
// f34 = T(-j)_hi
// f35 = T(-j)_lo

{ .mmi
(p0)  sub                  r34 = r35, r32    
(p0)  addl           r37   = @ltoff(double_sinh_j_table), gp
      nop.i 999
}
;;

{ .mmi
      ld8 r37 = [r37]
      nop.m 999
      nop.i 999
}
;;


{ .mfi
         nop.m 999
(p0)     fma.s1             sinh_FR_Rcub = sinh_FR_Rsq, sinh_FR_R, f0  
         nop.i 999
}

// ******************************************************
// STEP 3 Now decide if we need to branch to EXP
// ******************************************************
// Put 32 in f9; p6 true if x < 32
// Go to EXP if |x| >= 32 

{ .mlx
         nop.m 999
(p0)     movl                r32 = 0x0000000000010004 ;;               
}

// Calculate p_even
// f34 = B_2 + Rsq *B_3
// f35 = B_1 + Rsq*f34      = B_1 + Rsq * (B_2 + Rsq *B_3)
// f36 = p_even = Rsq * f35 = Rsq * (B_1 + Rsq * (B_2 + Rsq *B_3))

{ .mfi
         nop.m 999
(p0)     fma.s1          sinh_FR_peven_temp1 = sinh_FR_Rsq, sinh_FR_B3,          sinh_FR_B2  
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1          sinh_FR_peven_temp2 = sinh_FR_Rsq, sinh_FR_peven_temp1, sinh_FR_B1  
         nop.i 999
}

// Calculate p_odd
// f34 = A_2 + Rsq *A_3
// f35 = A_1 + Rsq * (A_2 + Rsq *A_3)
// f37 = podd = R + Rcub * (A_1 + Rsq * (A_2 + Rsq *A_3))

{ .mfi
         nop.m 999
(p0)     fma.s1          sinh_FR_podd_temp1 = sinh_FR_Rsq,        sinh_FR_A3,         sinh_FR_A2  
         nop.i 999 ;;
}

{ .mfi
(p0)     setf.exp            sinh_FR_N_temp1 = r39            
         nop.f 999
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1          sinh_FR_peven       = sinh_FR_Rsq, sinh_FR_peven_temp2, f0     
         nop.i 999
}

{ .mfi
         nop.m 999
(p0)     fma.s1          sinh_FR_podd_temp2 = sinh_FR_Rsq,        sinh_FR_podd_temp1, sinh_FR_A1  
         nop.i 999 ;;
}

{ .mfi
(p0)     setf.exp            f9  = r32                              
         nop.f 999
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1          sinh_FR_podd       = sinh_FR_podd_temp2, sinh_FR_Rcub,       sinh_FR_R   
         nop.i 999
}

// sinh_GR_mj contains the table offset for -j
// sinh_GR_j  contains the table offset for +j
// p6 is true when j <= 0

{ .mlx
(p0)     setf.exp            sinh_FR_N_temp2 = r40            
(p0)     movl                r40 = 0x0000000000000020 ;;    
}

{ .mfi
(p0)     sub                 sinh_GR_mJ = r40,  r36           
(p0)     fmerge.se           sinh_FR_spos    = sinh_FR_N_temp1, f1 
(p0)     adds                sinh_GR_J  = 0x20, r36 ;;           
}

{ .mii
         nop.m 999
(p0)     shl                  sinh_GR_mJ = sinh_GR_mJ, 5 ;;   
(p0)     add                  sinh_AD_mJ = r37, sinh_GR_mJ ;; 
}

{ .mmi
         nop.m 999
(p0)     ldfe                 sinh_FR_Tmjhi = [sinh_AD_mJ],16                 
(p0)     shl                  sinh_GR_J  = sinh_GR_J, 5 ;;    
}

{ .mfi
(p0)     ldfs                 sinh_FR_Tmjlo = [sinh_AD_mJ],16                 
(p0)     fcmp.lt.unc.s1      p0,p7 = sinh_FR_X,f9                          
(p0)     add                  sinh_AD_J  = r37, sinh_GR_J ;;  
}

{ .mmi
(p0)     ldfe                 sinh_FR_Tjhi  = [sinh_AD_J],16 ;;                  
(p0)     ldfs                 sinh_FR_Tjlo  = [sinh_AD_J],16                  
         nop.i 999 ;;
}

{ .mfb
         nop.m 999
(p0)     fmerge.se           sinh_FR_sneg    = sinh_FR_N_temp2, f1 
(p7)     br.cond.spnt        L(SINH_BY_EXP) ;;                            
}

{ .mfi
         nop.m 999
         nop.f 999
         nop.i 999 ;;
}

// ******************************************************
// If NOT branch to EXP
// ******************************************************
// Calculate S_hi and S_lo
// sinh_FR_S_hi_temp = sinh_FR_sneg * sinh_FR_Tmjhi
// sinh_FR_S_hi = sinh_FR_spos * sinh_FR_Tjhi - sinh_FR_S_hi_temp
// sinh_FR_S_hi = sinh_FR_spos * sinh_FR_Tjhi - (sinh_FR_sneg * sinh_FR_Tmjlo)

{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_S_hi_temp = sinh_FR_sneg, sinh_FR_Tmjhi, f0   
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)    fms.s1         sinh_FR_S_hi = sinh_FR_spos, sinh_FR_Tjhi,  sinh_FR_S_hi_temp              
         nop.i 999
}

// Calculate C_hi
// sinh_FR_C_hi_temp1 = sinh_FR_sneg * sinh_FR_Tmjhi
// sinh_FR_C_hi = sinh_FR_spos * sinh_FR_Tjhi + sinh_FR_C_hi_temp1

{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_C_hi_temp1 = sinh_FR_sneg, sinh_FR_Tmjhi, f0                   
         nop.i 999 ;;
}

// sinh_FR_S_lo_temp1 =  sinh_FR_spos * sinh_FR_Tjhi - sinh_FR_S_hi
// sinh_FR_S_lo_temp2 = -sinh_FR_sneg * sinh_FR_Tmjlo + (sinh_FR_spos * sinh_FR_Tjhi - sinh_FR_S_hi)
// sinh_FR_S_lo_temp2 = -sinh_FR_sneg * sinh_FR_Tmjlo + (sinh_FR_S_lo_temp1              )

{ .mfi
         nop.m 999
(p0)    fms.s1         sinh_FR_S_lo_temp1 =  sinh_FR_spos, sinh_FR_Tjhi,  sinh_FR_S_hi            
         nop.i 999
}

{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_C_hi       = sinh_FR_spos, sinh_FR_Tjhi, sinh_FR_C_hi_temp1    
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)    fnma.s1        sinh_FR_S_lo_temp2 = sinh_FR_sneg, sinh_FR_Tmjhi, sinh_FR_S_lo_temp1       
         nop.i 999
}

// sinh_FR_S_lo_temp1 = sinh_FR_sneg * sinh_FR_Tmjlo
// sinh_FR_S_lo_temp3 = sinh_FR_spos * sinh_FR_Tjlo - sinh_FR_S_lo_temp1
// sinh_FR_S_lo_temp3 = sinh_FR_spos * sinh_FR_Tjlo -(sinh_FR_sneg * sinh_FR_Tmjlo)
// sinh_FR_S_lo = sinh_FR_S_lo_temp3 + sinh_FR_S_lo_temp2

{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_S_lo_temp1 =  sinh_FR_sneg, sinh_FR_Tmjlo, f0                  
         nop.i 999 ;;
}

/////////// BUG FIX fma to fms -TK
{ .mfi
         nop.m 999
(p0)    fms.s1         sinh_FR_S_lo_temp3 =  sinh_FR_spos, sinh_FR_Tjlo,  sinh_FR_S_lo_temp1  
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_S_lo       =  sinh_FR_S_lo_temp3, f1,   sinh_FR_S_lo_temp2     
         nop.i 999 ;;
}

// Y_hi = S_hi 
// Y_lo = C_hi*p_odd + (S_hi*p_even + S_lo)
// sinh_FR_Y_lo_temp = sinh_FR_S_hi * sinh_FR_peven + sinh_FR_S_lo
// sinh_FR_Y_lo      = sinh_FR_C_hi * sinh_FR_podd + sinh_FR_Y_lo_temp

{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_Y_lo_temp  = sinh_FR_S_hi, sinh_FR_peven, sinh_FR_S_lo           
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_Y_lo       =  sinh_FR_C_hi, sinh_FR_podd, sinh_FR_Y_lo_temp      
         nop.i 999 ;;
}

// sinh_FR_SINH = Y_hi + Y_lo
// f8 = answer = sinh_FR_SGNX * sinh_FR_SINH

// Dummy multiply to generate inexact
{ .mfi
         nop.m 999
(p0)     fmpy.s0      sinh_FR_tmp = sinh_FR_all_ones, sinh_FR_all_ones
         nop.i 999
}
{ .mfi
         nop.m 999
(p0)    fma.s1         sinh_FR_SINH       =  sinh_FR_S_hi, f1, sinh_FR_Y_lo    
         nop.i 999 ;;
}

{ .mfb
         nop.m 999
(p0)    fma.s.s0       f8 = sinh_FR_SGNX, sinh_FR_SINH,f0                      
(p0)    br.ret.sptk     b0 ;;                          
}


L(SINH_BY_EXP): 

// When p7 is true,  we know that an overflow is not going to happen
// When p7 is false, we must check for possible overflow
// p7 is the over_SAFE flag
// Y_hi = Tjhi
// Y_lo = Tjhi * (p_odd + p_even) +Tjlo
// Scale = sign * 2^(N-1)
// sinh_FR_Y_lo =  sinh_FR_Tjhi * (sinh_FR_peven + sinh_FR_podd)
// sinh_FR_Y_lo =  sinh_FR_Tjhi * (sinh_FR_Y_lo_temp      )

{ .mfi
         nop.m 999
(p0)   fma.s1            sinh_FR_Y_lo_temp =  sinh_FR_peven, f1, sinh_FR_podd                   
         nop.i 999
}

// Now we are in EXP. This is the only path where an overflow is possible
// but not for certain. So this is the only path where over_SAFE has any use.
// r34 still has N-1
// There is a danger of double-extended overflow   if N-1 > 16382 = 0x3ffe
// There is a danger of double overflow            if N-1 > 0x3fe = 1022
// There is a danger of single overflow            if N-1 > 0x7e = 126
{ .mlx
         nop.m 999
(p0)   movl                r32          = 0x000000000000007e ;;                       
}

{ .mfi
(p0)   cmp.gt.unc          p0,p7        = r34, r32                                 
(p0)   fmerge.s          sinh_FR_SCALE     = sinh_FR_SGNX, sinh_FR_spos                         
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)   fma.s1            sinh_FR_Y_lo      =  sinh_FR_Tjhi,  sinh_FR_Y_lo_temp, sinh_FR_Tjlo    
         nop.i 999 ;;
}

// f8 = answer = scale * (Y_hi + Y_lo)
{ .mfi
         nop.m 999
(p0)   fma.s1            sinh_FR_SINH_temp = sinh_FR_Y_lo,  f1, sinh_FR_Tjhi       
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)   fma.s.s0          f44          = sinh_FR_SCALE,  sinh_FR_SINH_temp, f0      
         nop.i 999 ;;
}

// Dummy multiply to generate inexact
{ .mfi
         nop.m 999
(p7)     fmpy.s0      sinh_FR_tmp = sinh_FR_all_ones, sinh_FR_all_ones
         nop.i 999 ;;
}

// If over_SAFE is set, return
{ .mfb
       nop.m 999
(p7)   fmerge.s            f8 = f44,f44                                            
(p7)   br.ret.sptk     b0 ;;                          
}

// Else see if we overflowed
// S0 user supplied status
// S2 user supplied status + WRE + TD  (Overflows)
// If WRE is set then an overflow will not occur in EXP.
// The input value that would cause a register (WRE) value to overflow is about 2^15
// and this input would go into the HUGE path.
// Answer with WRE is in f43.

{ .mfi
         nop.m 999
(p0)   fsetc.s2            0x7F,0x42                                               
         nop.i 999;;
}

{ .mfi
         nop.m 999
(p0)   fma.s.s2            f43  = sinh_FR_SCALE,  sinh_FR_SINH_temp, f0                      
         nop.i 999 ;;
}

// 1007F => 1007F -FFFF = 80(true)
// 80 + 7F = FF, which is 1 more that the exponent of the largest
// double (FE). So 0 1007F 8000000000000000  is one ulp more than
// largest single in register bias
// Now  set p8 if the answer with WRE is greater than or equal this value
// Also set p9 if the answer with WRE is less than or equal to negative this value

{ .mlx
         nop.m 999
(p0)   movl                r32     = 0x0000000001007F ;;                              
}

{ .mmf
         nop.m 999
(p0)   setf.exp            f41 = r32                                               
(p0)   fsetc.s2            0x7F,0x40 ;;                                               
}

{ .mfi
         nop.m 999
(p0)   fcmp.ge.unc.s1 p8, p0 =  f43, f41                                           
         nop.i 999
}

{ .mfi
         nop.m 999
(p0)   fmerge.ns           f42 = f41, f41                                          
         nop.i 999 ;;
}

// The error tag for overflow is 128
{ .mii
         nop.m 999
         nop.i 999 ;;
(p8)   mov                 r47 = 128 ;;                                               
}

{ .mfb
         nop.m 999
(p0)   fcmp.le.unc.s1      p9, p0 =  f43, f42                                      
(p8)   br.cond.spnt L(SINH_ERROR_SUPPORT) ;;
}

{ .mii
         nop.m 999
         nop.i 999 ;;
(p9)   mov                 r47 = 128                                               
}

{ .mib
         nop.m 999
         nop.i 999
(p9)   br.cond.spnt L(SINH_ERROR_SUPPORT) ;;
}

// Dummy multiply to generate inexact
{ .mfi
         nop.m 999
(p0)     fmpy.s0      sinh_FR_tmp = sinh_FR_all_ones, sinh_FR_all_ones
         nop.i 999 ;;
}

{ .mfb
         nop.m 999
(p0)   fmerge.s            f8 = f44,f44                                            
(p0)   br.ret.sptk     b0 ;;                          
}

L(SINH_HUGE): 

// for SINH_HUGE, put 24000 in exponent; take sign from input; add 1
// SAFE: SAFE is always 0 for HUGE

{ .mlx
         nop.m 999
(p0)   movl                r32 = 0x0000000000015dbf ;;                                
}

{ .mfi
(p0)   setf.exp            f9  = r32                                               
         nop.f 999
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)   fma.s1              sinh_FR_signed_hi_lo = sinh_FR_SGNX, f9, f1                       
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)   fma.s.s0            f44 = sinh_FR_signed_hi_lo,  f9, f0                          
(p0)   mov                 r47 = 128                                               
}
.endp sinhf
ASM_SIZE_DIRECTIVE(sinhf)
#ifdef _LIBC
ASM_SIZE_DIRECTIVE(__ieee754_sinhf)
#endif

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
L(SINH_ERROR_SUPPORT):
.prologue

// (1)
{ .mfi
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
        nop.f 0
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
        stfs [GR_Parameter_Y] = f0,16         // STORE Parameter 2 on stack
        add GR_Parameter_X = 16,sp            // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                     // Save b0
};;

.body
// (3)
{ .mib
        stfs [GR_Parameter_X] = f8                     // STORE Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y   // Parameter 3 address
        nop.b 0                            
}
{ .mib
        stfs [GR_Parameter_Y] = f44                    // STORE Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#          // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;

// (4)
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
