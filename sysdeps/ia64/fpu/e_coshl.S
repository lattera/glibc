.file "coshl.s"

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
// 1/23/01  Set inexact flag for large args.
//
// API
//==============================================================
// float       = cosh(float)
// double      = cosh(double)
// long double = coshl(long double)
// input  floating point f8
// output floating point f8


// Overview of operation
//==============================================================
// There are four paths

// 1. |x| < 0.25        COSH_BY_POLY
// 2. |x| < 32          COSH_BY_TBL
// 3. |x| < 2^14        COSH_BY_EXP
// 4. |x| >= 2^14       COSH_HUGE

// For paths 1, and 2 SAFE is always 1.
// For path  4, Safe is always 0.
// SAFE = 1 means we cannot overflow.

#include "libm_support.h"

// Assembly macros
//==============================================================
cosh_FR_X            = f44
FR_RESULT            = f44
cosh_FR_SGNX         = f40
cosh_FR_all_ones     = f45

FR_X                 = f8
FR_Y                 = f0
cosh_FR_Inv_log2by64 = f9
cosh_FR_log2by64_lo  = f11
cosh_FR_log2by64_hi  = f10

cosh_FR_A1           = f9
cosh_FR_A2           = f10
cosh_FR_A3           = f11

cosh_FR_Rcub         = f12
cosh_FR_M_temp       = f13
cosh_FR_R_temp       = f13
cosh_FR_Rsq          = f13
cosh_FR_R            = f14

cosh_FR_M            = f38

cosh_FR_tmp          = f15
cosh_FR_B1           = f15
cosh_FR_B2           = f32
cosh_FR_B3           = f33

cosh_FR_peven_temp1  = f34
cosh_FR_peven_temp2  = f35
cosh_FR_peven        = f36

cosh_FR_podd_temp1   = f34
cosh_FR_podd_temp2   = f35
cosh_FR_podd         = f37

cosh_FR_J_temp       = f9
cosh_FR_J            = f10

cosh_FR_Mmj          = f39

cosh_FR_N_temp1      = f11
cosh_FR_N_temp2      = f12
cosh_FR_N            = f13

cosh_FR_spos         = f14
cosh_FR_sneg         = f15

cosh_FR_Tjhi         = f32
cosh_FR_Tjlo         = f33
cosh_FR_Tmjhi        = f34
cosh_FR_Tmjlo        = f35

GR_mJ           = r35
GR_J            = r36

AD_mJ           = r38
AD_J            = r39

cosh_GR_all_ones     = r40

GR_SAVE_PFS           = r41
GR_SAVE_B0            = r42
GR_SAVE_GP            = r43
GR_Parameter_X        = r44
GR_Parameter_Y        = r45
GR_Parameter_RESULT   = r46
GR_Parameter_TAG      = r47 

cosh_FR_C_hi         = f9
cosh_FR_C_hi_temp    = f10
cosh_FR_C_lo_temp1   = f11 
cosh_FR_C_lo_temp2   = f12 
cosh_FR_C_lo_temp3   = f13 

cosh_FR_C_lo         = f38
cosh_FR_S_hi         = f39

cosh_FR_S_hi_temp1   = f10
cosh_FR_Y_hi         = f11 
cosh_FR_Y_lo_temp    = f12 
cosh_FR_Y_lo         = f13 
cosh_FR_COSH         = f9

cosh_FR_X2           = f9
cosh_FR_X4           = f10

cosh_FR_P1           = f14
cosh_FR_P2           = f15
cosh_FR_P3           = f32
cosh_FR_P4           = f33
cosh_FR_P5           = f34
cosh_FR_P6           = f35

cosh_FR_TINY_THRESH  = f9

cosh_FR_COSH_temp    = f10
cosh_FR_SCALE        = f11 

cosh_FR_hi_lo = f10

cosh_FR_poly_podd_temp1    =  f11 
cosh_FR_poly_podd_temp2    =  f13
cosh_FR_poly_peven_temp1   =  f11
cosh_FR_poly_peven_temp2   =  f13

// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16
double_cosh_arg_reduction:
ASM_TYPE_DIRECTIVE(double_cosh_arg_reduction,@object)
   data8 0xB8AA3B295C17F0BC, 0x00004005
   data8 0xB17217F7D1000000, 0x00003FF8
   data8 0xCF79ABC9E3B39804, 0x00003FD0
ASM_SIZE_DIRECTIVE(double_cosh_arg_reduction)

double_cosh_p_table:
ASM_TYPE_DIRECTIVE(double_cosh_p_table,@object)
   data8 0x8000000000000000, 0x00003FFE
   data8 0xAAAAAAAAAAAAAB80, 0x00003FFA
   data8 0xB60B60B60B4FE884, 0x00003FF5
   data8 0xD00D00D1021D7370, 0x00003FEF
   data8 0x93F27740C0C2F1CC, 0x00003FE9
   data8 0x8FA02AC65BCBD5BC, 0x00003FE2
ASM_SIZE_DIRECTIVE(double_cosh_p_table)

double_cosh_ab_table:
ASM_TYPE_DIRECTIVE(double_cosh_ab_table,@object)
   data8 0xAAAAAAAAAAAAAAAC, 0x00003FFC
   data8 0x88888888884ECDD5, 0x00003FF8
   data8 0xD00D0C6DCC26A86B, 0x00003FF2
   data8 0x8000000000000002, 0x00003FFE
   data8 0xAAAAAAAAAA402C77, 0x00003FFA
   data8 0xB60B6CC96BDB144D, 0x00003FF5
ASM_SIZE_DIRECTIVE(double_cosh_ab_table)

double_cosh_j_table:
ASM_TYPE_DIRECTIVE(double_cosh_j_table,@object)
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
ASM_SIZE_DIRECTIVE(double_cosh_j_table)

.align 32
.global coshl#

.section .text
.proc  coshl#
.align 32

coshl:

#ifdef _LIBC
.global __ieee754_coshl#
.proc __ieee754_coshl#
__ieee754_coshl:
#endif

// X NAN?

{ .mfi
      alloc r32 = ar.pfs,0,12,4,0                  
(p0)  fclass.m.unc  p6,p7 = f8, 0xc3               
      mov cosh_GR_all_ones = -1
};;

//   This is more than we need but it is in preparation
//   for the values we add for error support. We push three
//   addresses on the stack (3*8) = 24 bytes and one tag

{ .mfb
      nop.m 999
(p6)     fma.s0   f8 = f8,f1,f8                  
(p6)  br.ret.spnt     b0 ;;                          
}


// Make constant that will generate inexact when squared
// X infinity 
{ .mfi
      setf.sig cosh_FR_all_ones = cosh_GR_all_ones 
(p0)  fclass.m.unc  p6,p0 = f8, 0x23               
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p6)     fmerge.s      f8 = f0,f8                  
(p6)  br.ret.spnt     b0 ;;
}



// Put 0.25 in f9; p6 true if x < 0.25
{ .mlx
         nop.m 999
(p0)     movl            r32 = 0x000000000000fffd ;;         
}

{ .mfi
(p0)  setf.exp        f9 = r32                         
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fmerge.s      cosh_FR_X    = f0,f8                
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fmerge.s      cosh_FR_SGNX = f8,f1                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fcmp.lt.unc     p0,p7 = cosh_FR_X,f9                    
      nop.i 999 ;;
}

{ .mib
      nop.m 999
      nop.i 999
(p7)  br.cond.sptk    L(COSH_BY_TBL) 
}
;;


// COSH_BY_POLY: 
// POLY cannot overflow so there is no need to call __libm_error_support
// Get the values of P_x from the table

{ .mmi
      nop.m 999
(p0)  addl           r34   = @ltoff(double_cosh_p_table), gp
      nop.i 999
}
;;

{ .mmi
      ld8 r34 = [r34]
      nop.m 999
      nop.i 999
}
;;


// Calculate cosh_FR_X2 = ax*ax and cosh_FR_X4 = ax*ax*ax*ax
{ .mmf
         nop.m 999
(p0)     ldfe       cosh_FR_P1 = [r34],16                 
(p0)     fma.s1     cosh_FR_X2 = cosh_FR_X, cosh_FR_X, f0 ;;           
}

{ .mmi
(p0)     ldfe       cosh_FR_P2 = [r34],16 ;;                 
(p0)     ldfe       cosh_FR_P3 = [r34],16                 
         nop.i 999 ;;
}

{ .mmi
(p0)     ldfe       cosh_FR_P4 = [r34],16 ;;                 
(p0)     ldfe       cosh_FR_P5 = [r34],16                 
         nop.i 999 ;;
}

{ .mfi
(p0)     ldfe       cosh_FR_P6 = [r34],16                 
(p0)     fma.s1     cosh_FR_X4 = cosh_FR_X2, cosh_FR_X2, f0         
         nop.i 999 ;;
}

// Calculate cosh_FR_podd = x4 *(x4 * P_5 + P_3) + P_1
{ .mfi
         nop.m 999
(p0)     fma.s1     cosh_FR_poly_podd_temp1 = cosh_FR_X4, cosh_FR_P5, cosh_FR_P3                
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1     cosh_FR_podd            = cosh_FR_X4, cosh_FR_poly_podd_temp1, cosh_FR_P1   
         nop.i 999
}

// Calculate cosh_FR_peven =  p_even = x4 *(x4 * (x4 * P_6 + P_4) + P_2)
{ .mfi
         nop.m 999
(p0)     fma.s1     cosh_FR_poly_peven_temp1 = cosh_FR_X4, cosh_FR_P6, cosh_FR_P4               
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1     cosh_FR_poly_peven_temp2 = cosh_FR_X4, cosh_FR_poly_peven_temp1, cosh_FR_P2 
         nop.i 999 ;;
}

{ .mfi
         nop.m 999
(p0)     fma.s1     cosh_FR_peven       = cosh_FR_X4, cosh_FR_poly_peven_temp2, f0         
         nop.i 999 ;;
}

// Y_lo = x2*p_odd + p_even
// Calculate f8 = Y_hi + Y_lo 
{ .mfi
         nop.m 999
(p0)     fma.s1     cosh_FR_Y_lo         = cosh_FR_X2, cosh_FR_podd,  cosh_FR_peven    
         nop.i 999 ;;
}

{ .mfb
         nop.m 999
(p0)     fma.s0   f8                   = f1, f1, cosh_FR_Y_lo                        
(p0)     br.ret.sptk     b0 ;;
}


L(COSH_BY_TBL): 

// Now that we are at TBL; so far all we know is that |x| >= 0.25.
// The first two steps are the same for TBL and EXP, but if we are HUGE
// Double Extended
// Go to HUGE if |x| >= 2^14, 1000d (register-biased) is e = 14 (true)
// Double
// Go to HUGE if |x| >= 2^10, 10009 (register-biased) is e = 10 (true)
// Single
// Go to HUGE if |x| >= 2^7,  10006 (register-biased) is e =  7 (true)
// we want to leave now. Go to HUGE if |x| >= 2^14
// 1000d (register-biased) is e = 14 (true)

{ .mlx
      nop.m 999
(p0)     movl            r32 = 0x000000000001000d ;;              
}

{ .mfi
(p0)     setf.exp        f9 = r32                              
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)     fcmp.ge.unc     p6,p7 = cosh_FR_X,f9                  
      nop.i 999 ;;
}

{ .mib
      nop.m 999
      nop.i 999
(p6)     br.cond.spnt    L(COSH_HUGE) ;;                             
}

// r32 = 1
// r34 = N-1 
// r35 = N
// r36 = j
// r37 = N+1

// TBL can never overflow
// cosh(x) = cosh(B+R)
//         = cosh(B) cosh(R) + sinh(B) sinh(R) 
// cosh(R) can be approximated by 1 + p_even
// sinh(R) can be approximated by p_odd

// ******************************************************
// STEP 1 (TBL and EXP)
// ******************************************************
// Get the following constants.
// f9  = Inv_log2by64
// f10 = log2by64_hi
// f11 = log2by64_lo

{ .mmi
(p0)     adds                 r32 = 0x1,r0      
(p0)     addl           r34   = @ltoff(double_cosh_arg_reduction), gp
         nop.i 999
}
;;

// We want 2^(N-1) and 2^(-N-1). So bias N-1 and -N-1 and
// put them in an exponent.
// cosh_FR_spos = 2^(N-1) and cosh_FR_sneg = 2^(-N-1)
// r39 = 0xffff + (N-1)  = 0xffff +N -1
// r40 = 0xffff - (N +1) = 0xffff -N -1

{ .mlx
         ld8 r34 = [r34]
(p0)     movl                r38 = 0x000000000000fffe ;; 
}

{ .mmi
(p0)     ldfe            cosh_FR_Inv_log2by64 = [r34],16 ;;            
(p0)     ldfe            cosh_FR_log2by64_hi  = [r34],16            
         nop.i 999 ;;
}

{ .mbb
(p0)     ldfe            cosh_FR_log2by64_lo  = [r34],16            
         nop.b 999
         nop.b 999 ;;
}

// Get the A coefficients
// f9  = A_1
// f10 = A_2
// f11 = A_3

{ .mmi
      nop.m 999
(p0)  addl           r34   = @ltoff(double_cosh_ab_table), gp
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
// M = round-to-integer(x*Inv_log2by64)
// cosh_FR_M = M = truncate(ax/(log2/64))
// Put the significand of M in r35
//    and the floating point representation of M in cosh_FR_M

{ .mfi
      nop.m 999
(p0)  fma.s1          cosh_FR_M      = cosh_FR_X, cosh_FR_Inv_log2by64, f0 
      nop.i 999
}

{ .mfi
(p0)  ldfe            cosh_FR_A1 = [r34],16            
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fcvt.fx.s1      cosh_FR_M_temp = cosh_FR_M                      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnorm.s1        cosh_FR_M      = cosh_FR_M_temp                 
      nop.i 999 ;;
}

{ .mfi
(p0)  getf.sig        r35       = cosh_FR_M_temp                 
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
(p0)  and            r36 = 0x3f, r35 ;;   
}

// Calculate R
// f13 = f44 - f12*f10 = x - M*log2by64_hi
// f14 = f13 - f8*f11 = R = (x - M*log2by64_hi) - M*log2by64_lo

{ .mfi
      nop.m 999
(p0)  fnma.s1        cosh_FR_R_temp = cosh_FR_M, cosh_FR_log2by64_hi, cosh_FR_X      
      nop.i 999
}

{ .mfi
(p0)  ldfe            cosh_FR_A2 = [r34],16            
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        cosh_FR_R      = cosh_FR_M, cosh_FR_log2by64_lo, cosh_FR_R_temp 
      nop.i 999
}

// Get the B coefficients
// f15 = B_1
// f32 = B_2
// f33 = B_3

{ .mmi
(p0)     ldfe            cosh_FR_A3 = [r34],16 ;;            
(p0)     ldfe            cosh_FR_B1 = [r34],16            
         nop.i 999 ;;
}

{ .mmi
(p0)     ldfe            cosh_FR_B2 = [r34],16 ;;            
(p0)     ldfe            cosh_FR_B3 = [r34],16            
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
(p0)     fma.s1          cosh_FR_Rsq  = cosh_FR_R,   cosh_FR_R, f0  
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
(p0)     sub                  r34 = r35, r32    
(p0)     addl    r37   = @ltoff(double_cosh_j_table), gp
         nop.i 999
}
;;

{ .mfi
      ld8 r37 = [r37]
(p0)  fma.s1          cosh_FR_Rcub = cosh_FR_Rsq, cosh_FR_R, f0  
      nop.i 999
}

// ******************************************************
// STEP 3 Now decide if we need to branch to EXP
// ******************************************************
// Put 32 in f9; p6 true if x < 32

{ .mlx
         nop.m 999
(p0)     movl                r32 = 0x0000000000010004 ;;               
}

// Calculate p_even
// f34 = B_2 + Rsq *B_3
// f35 = B_1 + Rsq*f34      = B_1 + Rsq * (B_2 + Rsq *B_3)
// f36 = peven = Rsq * f35 = Rsq * (B_1 + Rsq * (B_2 + Rsq *B_3))

{ .mfi
      nop.m 999
(p0)  fma.s1          cosh_FR_peven_temp1 = cosh_FR_Rsq, cosh_FR_B3,          cosh_FR_B2  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1          cosh_FR_peven_temp2 = cosh_FR_Rsq, cosh_FR_peven_temp1, cosh_FR_B1  
      nop.i 999
}

// Calculate p_odd
// f34 = A_2 + Rsq *A_3
// f35 = A_1 + Rsq * (A_2 + Rsq *A_3)
// f37 = podd = R + Rcub * (A_1 + Rsq * (A_2 + Rsq *A_3))

{ .mfi
      nop.m 999
(p0)  fma.s1          cosh_FR_podd_temp1 = cosh_FR_Rsq,        cosh_FR_A3,         cosh_FR_A2  
      nop.i 999 ;;
}

{ .mfi
(p0)  setf.exp            cosh_FR_N_temp1 = r39            
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1          cosh_FR_peven       = cosh_FR_Rsq, cosh_FR_peven_temp2, f0     
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fma.s1          cosh_FR_podd_temp2 = cosh_FR_Rsq,        cosh_FR_podd_temp1, cosh_FR_A1  
      nop.i 999 ;;
}

{ .mfi
(p0)  setf.exp            f9  = r32                              
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1          cosh_FR_podd       = cosh_FR_podd_temp2, cosh_FR_Rcub,       cosh_FR_R   
      nop.i 999
}

// sinh_GR_mj contains the table offset for -j
// sinh_GR_j  contains the table offset for +j
// p6 is true when j <= 0

{ .mlx
(p0)     setf.exp            cosh_FR_N_temp2 = r40            
(p0)     movl                r40 = 0x0000000000000020 ;;    
}

{ .mfi
(p0)     sub                 GR_mJ = r40,  r36           
(p0)     fmerge.se           cosh_FR_spos    = cosh_FR_N_temp1, f1 
(p0)     adds                GR_J  = 0x20, r36 ;;           
}

{ .mii
         nop.m 999
(p0)     shl                  GR_mJ = GR_mJ, 5 ;;   
(p0)     add                  AD_mJ = r37, GR_mJ ;; 
}

{ .mmi
         nop.m 999
(p0)     ldfe                 cosh_FR_Tmjhi = [AD_mJ],16                 
(p0)     shl                  GR_J  = GR_J, 5 ;;    
}

{ .mfi
(p0)     ldfs                 cosh_FR_Tmjlo = [AD_mJ],16                 
(p0)     fcmp.lt.unc.s1      p6,p7 = cosh_FR_X,f9                          
(p0)     add                  AD_J  = r37, GR_J ;;  
}

{ .mmi
(p0)     ldfe                 cosh_FR_Tjhi  = [AD_J],16 ;;                  
(p0)     ldfs                 cosh_FR_Tjlo  = [AD_J],16                  
         nop.i 999 ;;
}

{ .mfb
         nop.m 999
(p0)     fmerge.se           cosh_FR_sneg    = cosh_FR_N_temp2, f1 
(p7)     br.cond.spnt        L(COSH_BY_EXP) ;;                            
}

// ******************************************************
// If NOT branch to EXP
// ******************************************************
// Calculate C_hi
// ******************************************************
// cosh_FR_C_hi_temp = cosh_FR_sneg * cosh_FR_Tmjhi
// cosh_FR_C_hi = cosh_FR_spos * cosh_FR_Tjhi + (cosh_FR_sneg * cosh_FR_Tmjhi)

{ .mfi
      nop.m 999
(p0)  fma.s1         cosh_FR_C_hi_temp = cosh_FR_sneg, cosh_FR_Tmjhi, f0                   
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         cosh_FR_C_hi      = cosh_FR_spos, cosh_FR_Tjhi,  cosh_FR_C_hi_temp    
      nop.i 999
}

// ******************************************************
// Calculate S_hi
// ******************************************************
// cosh_FR_S_hi_temp1 = cosh_FR_sneg * cosh_FR_Tmjhi
// cosh_FR_S_hi = cosh_FR_spos * cosh_FR_Tjhi - cosh_FR_C_hi_temp1

{ .mfi
      nop.m 999
(p0)  fma.s1        cosh_FR_S_hi_temp1 =  cosh_FR_sneg, cosh_FR_Tmjhi, f0                
      nop.i 999 ;;
}

// ******************************************************
// Calculate C_lo
// ******************************************************
// cosh_FR_C_lo_temp1 = cosh_FR_spos * cosh_FR_Tjhi - cosh_FR_C_hi
// cosh_FR_C_lo_temp2 = cosh_FR_sneg * cosh_FR_Tmjlo + (cosh_FR_spos * cosh_FR_Tjhi - cosh_FR_C_hi)
// cosh_FR_C_lo_temp1 = cosh_FR_sneg * cosh_FR_Tmjlo
// cosh_FR_C_lo_temp3 = cosh_FR_spos * cosh_FR_Tjlo + (cosh_FR_sneg * cosh_FR_Tmjlo)
// cosh_FR_C_lo = cosh_FR_C_lo_temp3 + cosh_FR_C_lo_temp2

{ .mfi
      nop.m 999
(p0)  fms.s1        cosh_FR_C_lo_temp1 = cosh_FR_spos, cosh_FR_Tjhi,  cosh_FR_C_hi        
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fms.s1        cosh_FR_S_hi       =  cosh_FR_spos, cosh_FR_Tjhi, cosh_FR_S_hi_temp1 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1        cosh_FR_C_lo_temp2 = cosh_FR_sneg, cosh_FR_Tmjhi, cosh_FR_C_lo_temp1  
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fma.s1        cosh_FR_C_lo_temp1 = cosh_FR_sneg, cosh_FR_Tmjlo, f0                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1        cosh_FR_C_lo_temp3 =  cosh_FR_spos, cosh_FR_Tjlo,  cosh_FR_C_lo_temp1 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1        cosh_FR_C_lo       =  cosh_FR_C_lo_temp3, f1,   cosh_FR_C_lo_temp2    
      nop.i 999 ;;
}

// ******************************************************
// cosh_FR_Y_lo_temp = cosh_FR_C_hi * cosh_FR_peven + cosh_FR_C_lo
// cosh_FR_Y_lo = cosh_FR_S_hi * cosh_FR_podd + cosh_FR_Y_lo_temp
// cosh_FR_COSH = Y_hi + Y_lo

{ .mfi
      nop.m 999
(p0)  fma.s1         cosh_FR_Y_lo_temp =  cosh_FR_C_hi, cosh_FR_peven, cosh_FR_C_lo       
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         cosh_FR_Y_lo      =  cosh_FR_S_hi, cosh_FR_podd, cosh_FR_Y_lo_temp   
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p0)  fma.s0       f8 =  cosh_FR_C_hi, f1, cosh_FR_Y_lo                       
(p0)  br.ret.sptk     b0 ;;
}

L(COSH_BY_EXP): 

// When p7 is true,  we know that an overflow is not going to happen
// When p7 is false, we must check for possible overflow
// p7 is the over_SAFE flag
// f44 = Scale * (Y_hi + Y_lo)
//     =  cosh_FR_spos * (cosh_FR_Tjhi + cosh_FR_Y_lo)

{ .mfi
      nop.m 999
(p0)  fma.s1         cosh_FR_Y_lo_temp =  cosh_FR_peven, f1,       cosh_FR_podd           
      nop.i 999
}

// Now we are in EXP. This is the only path where an overflow is possible
// but not for certain. So this is the only path where over_SAFE has any use.
// r34 still has N-1
// There is a danger of double-extended overflow   if N-1 > 0x3ffe = 16382
// There is a danger of double overflow            if N-1 > 0x3fe  = 1022
// There is a danger of single overflow            if N-1 > 0x7e   = 126

{ .mlx
       nop.m 999
(p0)   movl                r32          = 0x0000000000003ffe ;;                       
}

{ .mfi
(p0)  cmp.gt.unc          p0,p7        = r34, r32                                 
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         cosh_FR_Y_lo      =  cosh_FR_Tjhi,  cosh_FR_Y_lo_temp, cosh_FR_Tjlo       
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         cosh_FR_COSH_temp =  cosh_FR_Y_lo,  f1, cosh_FR_Tjhi                 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s0       f44 = cosh_FR_spos,  cosh_FR_COSH_temp, f0                       
      nop.i 999 ;;
}

// Dummy multiply to generate inexact
{ .mfi
         nop.m 999
(p7)     fmpy.s0      cosh_FR_tmp = cosh_FR_all_ones, cosh_FR_all_ones
         nop.i 999 ;;
}

// If over_SAFE is set, return
{ .mfb
       nop.m 999
(p7)   fmerge.s            f8 = f44,f44
(p7)  br.ret.sptk     b0 ;;
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
(p0)  fsetc.s2            0x7F,0x42                                               
      nop.i 999;;
}

{ .mfi
      nop.m 999
(p0)  fma.s2            f43  = cosh_FR_spos,  cosh_FR_COSH_temp, f0                      
      nop.i 999 ;;
}

// 103FF => 103FF -FFFF = 400(true)
// 400 + 3FF = 7FF, which is 1 more than the exponent of the largest
// double (7FE). So 0 103FF 8000000000000000  is one ulp more than
// largest double in register bias

// 13FFF => 13FFF -FFFF = 4000(true)

// Now  set p8 if the answer with WRE is greater than or equal this value
// Also set p9 if the answer with WRE is less than or equal to negative this value

{ .mlx
       nop.m 999
(p0)   movl                r32          = 0x0000000000013fff ;;                     
}

{ .mmf
       nop.m 999
(p0)   setf.exp            f41          = r32                                    
(p0)   fsetc.s2            0x7F,0x40 ;;                                               
}

{ .mfi
      nop.m 999
(p0)  fcmp.ge.unc.s1      p8, p0       = f43, f41                               
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fmerge.ns           f42 = f41, f41                                          
      nop.i 999 ;;
}

// The error tag for overflow is 63
{ .mii
      nop.m 999
      nop.i 999 ;;
(p8)  mov                 GR_Parameter_TAG = 63 ;;                                               
}

{ .mfb
      nop.m 999
(p0)  fcmp.le.unc.s1      p9, p0 =  f43, f42                                      
(p8)  br.cond.spnt __libm_error_region ;;
}

{ .mii
      nop.m 999
      nop.i 999 ;;
(p9)  mov                 GR_Parameter_TAG = 63                                               
}

{ .mib
      nop.m 999
      nop.i 999
(p9)  br.cond.spnt __libm_error_region ;;
}

// Dummy multiply to generate inexact
{ .mfi
         nop.m 999
(p0)     fmpy.s0      cosh_FR_tmp = cosh_FR_all_ones, cosh_FR_all_ones
         nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p0)  fmerge.s            f8 = f44,f44                                            
(p0)  br.ret.sptk     b0 ;;
}


// for COSH_HUGE, put 24000 in exponent; take sign from input; add 1
// SAFE: SAFE is always 0 for HUGE

L(COSH_HUGE): 

{ .mlx
      nop.m 999
(p0)  movl                r32 = 0x0000000000015dbf ;;                                
}

{ .mfi
(p0)  setf.exp            f9  = r32                                               
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1              cosh_FR_hi_lo = f1, f9, f1                              
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s0            f44 = f9, cosh_FR_hi_lo, f0                             
(p0)  mov                 GR_Parameter_TAG = 63                                               
}
.endp coshl
ASM_SIZE_DIRECTIVE(coshl)

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
        stfe [GR_Parameter_Y] = FR_Y,16         // Save Parameter 2 on stack
        add GR_Parameter_X = 16,sp              // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                       // Save b0
};;
.body
{ .mib
        stfe [GR_Parameter_X] = FR_X            // Store Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y
        nop.b 0                                 // Parameter 3 address
}
{ .mib
        stfe [GR_Parameter_Y] = FR_RESULT      // Store Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#  // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;
{ .mmi
        ldfe  f8 = [GR_Parameter_RESULT]       // Get return result off stack
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
