.file "asin.s"

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
// 2/02/00  Initial version 
// 8/17/00  New and much faster algorithm.
// 8/31/00  Avoided bank conflicts on loads, shortened |x|=1 path, 
//          fixed mfb split issue stalls.
// 12/19/00 Fixed small arg cases to force inexact, or inexact and underflow.

// Description
//=========================================
// The asin function computes the principle value of the arc sine of x.
// asin(0) returns 0, asin(1) returns pi/2, asin(-1) returns -pi/2.
// A doman error occurs for arguments not in the range [-1,+1].

// The asin function returns the arc sine in the range [-pi/2, +pi/2] radians.

#include "libm_support.h"

//
// Assembly macros
//=========================================


// predicate registers
//asin_pred_LEsqrt2by2            = p7
//asin_pred_GTsqrt2by2            = p8

// integer registers
ASIN_Addr1                      = r33
ASIN_Addr2                      = r34
ASIN_FFFE                       = r35
ASIN_lnorm_sig                  = r36
ASIN_snorm_exp                  = r37

GR_SAVE_B0                      = r36
GR_SAVE_PFS                     = r37
GR_SAVE_GP                      = r38

GR_Parameter_X                  = r39
GR_Parameter_Y                  = r40
GR_Parameter_RESULT             = r41
GR_Parameter_Tag                = r42

// floating point registers
asin_coeff_P1                   = f32
asin_coeff_P2                   = f33
asin_coeff_P3                   = f34
asin_coeff_P4                   = f35

asin_coeff_P5                   = f36
asin_coeff_P6                   = f37
asin_coeff_P7                   = f38
asin_coeff_P8                   = f39
asin_coeff_P9                   = f40

asin_coeff_P10                  = f41
asin_coeff_P11                  = f42
asin_coeff_P12                  = f43
asin_coeff_P13                  = f44
asin_coeff_P14                  = f45

asin_coeff_P15                  = f46
asin_coeff_P16                  = f47
asin_coeff_P17                  = f48
asin_coeff_P18                  = f49
asin_coeff_P19                  = f50

asin_coeff_P20                  = f51
asin_coeff_P21                  = f52
asin_const_sqrt2by2             = f53
asin_const_piby2                = f54
asin_abs_x                      = f55

asin_tx                         = f56
asin_tx2                        = f57
asin_tx3                        = f58
asin_tx4                        = f59
asin_tx8                        = f60

asin_tx11                       = f61
asin_1poly_p8                   = f62
asin_1poly_p19                  = f63
asin_1poly_p4                   = f64
asin_1poly_p15                  = f65

asin_1poly_p6                   = f66
asin_1poly_p17                  = f67
asin_1poly_p0                   = f68
asin_1poly_p11                  = f69
asin_1poly_p2                   = f70

asin_1poly_p13                  = f71
asin_series_tx                  = f72
asin_t                          = f73
asin_t2                         = f74
asin_t3                         = f75

asin_t4                         = f76
asin_t8                         = f77
asin_t11                        = f78
asin_poly_p8                    = f79
asin_poly_p19                   = f80

asin_poly_p4                    = f81
asin_poly_p15                   = f82
asin_poly_p6                    = f83
asin_poly_p17                   = f84
asin_poly_p0                    = f85

asin_poly_p11                   = f86
asin_poly_p2                    = f87
asin_poly_p13                   = f88
asin_series_t                   = f89
asin_1by2                       = f90

asin_3by2                       = f91
asin_5by2                       = f92
asin_11by4                      = f93
asin_35by8                      = f94
asin_63by8                      = f95

asin_231by16                    = f96 
asin_y0                         = f97 
asin_H0                         = f98 
asin_S0                         = f99 
asin_d                          = f100

asin_l1                         = f101
asin_d2                         = f102
asin_T0                         = f103
asin_d1                         = f104
asin_e0                         = f105

asin_l2                         = f106
asin_d3                         = f107
asin_T3                         = f108
asin_S1                         = f109
asin_e1                         = f110

asin_z                          = f111
answer2                         = f112
asin_sgn_x                      = f113
asin_429by16                    = f114
asin_18by4                      = f115

asin_3by4                       = f116
asin_l3                         = f117
asin_T6                         = f118
asin_eps_exp                    = f119
asin_eps_sig                    = f120
asin_eps                        = f120

// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

asin_coeff_1_table:
ASM_TYPE_DIRECTIVE(asin_coeff_1_table,@object)
data8 0xE4E7E0A423A21249  , 0x00003FF8 //P7
data8 0xC2F7EE0200FCE2A5  , 0x0000C003 //P18
data8 0xB745D7F6C65C20E0  , 0x00003FF9 //P5
data8 0xF75E381A323D4D94  , 0x0000C002 //P16
data8 0x8959C2629C1024C0  , 0x0000C002 //P20
data8 0xAFF68E7D241292C5  , 0x00003FF8 //P9
data8 0xB6DB6DB7260AC30D  , 0x00003FFA //P3
data8 0xD0417CE2B41CB7BF  , 0x0000C000 //P14
data8 0x81D570FEA724E3E4  , 0x0000BFFD //P12
data8 0xAAAAAAAAAAAAC277  , 0x00003FFC //P1
data8 0xF534912FF3E7B76F  , 0x00003FFF //P21
data8 0xc90fdaa22168c235  , 0x00003fff // pi/2
data8 0x0000000000000000  , 0x00000000 // pad to avoid data bank conflict
ASM_SIZE_DIRECTIVE(asin_coeff_1_table)
	

asin_coeff_2_table:
ASM_TYPE_DIRECTIVE(asin_coeff_2_table,@object)
data8 0x8E26AF5F29B39A2A  , 0x00003FF9 //P6
data8 0xB4F118A4B1015470  , 0x00004003 //P17
data8 0xF8E38E10C25990E0  , 0x00003FF9 //P4
data8 0x80F50489AEF1CAC6  , 0x00004002 //P15
data8 0x92728015172CFE1C  , 0x00004003 //P19
data8 0xBBC3D831D4595971  , 0x00003FF8 //P8
data8 0x999999999952A5C3  , 0x00003FFB //P2
data8 0x855576BE6F0975EC  , 0x00003FFF //P13
data8 0xF12420E778077D89  , 0x00003FFA //P11
data8 0xB6590FF4D23DE003  , 0x00003FF3 //P10
data8 0xb504f333f9de6484  , 0x00003ffe // sqrt(2)/2
ASM_SIZE_DIRECTIVE(asin_coeff_2_table)



.align 32
.global asin

.section .text
.proc  asin
.align 32


asin:
 
{     .mfi 
     alloc      r32               = ar.pfs,1,6,4,0
     fma.s1    asin_tx        =    f8,f8,f0
     addl      ASIN_Addr2     =    @ltoff(asin_coeff_2_table),gp
} 
{     .mfi 
     mov       ASIN_FFFE      =    0xFFFE
     fnma.s1   asin_t         =    f8,f8,f1
     addl      ASIN_Addr1     =    @ltoff(asin_coeff_1_table),gp
}
;;

 
{     .mfi 
     setf.exp       asin_1by2      =    ASIN_FFFE
     fmerge.s       asin_abs_x     =    f1,f8
     nop.i          999              ;;
} 
 
{     .mmf 
     ld8       ASIN_Addr1     =    [ASIN_Addr1]
     ld8       ASIN_Addr2     =    [ASIN_Addr2]
     fmerge.s  asin_sgn_x     =    f8,f1 ;;
} 

 
{     .mfi 
     ldfe      asin_coeff_P7  =    [ASIN_Addr1],16
     fma.s1    asin_tx2       =    asin_tx,asin_tx,f0
     nop.i                      999
} 
{     .mfi 
     ldfe      asin_coeff_P6  =    [ASIN_Addr2],16
     fma.s1    asin_t2        =    asin_t,asin_t,f0
     nop.i                      999;;
}

 
{     .mmf 
     ldfe      asin_coeff_P18 =    [ASIN_Addr1],16
     ldfe      asin_coeff_P17 =    [ASIN_Addr2],16
     fclass.m.unc p8,p0  = f8, 0xc3	//@qnan |@snan
} 
;;
 
{     .mmf 
     ldfe           asin_coeff_P5  =    [ASIN_Addr1],16
     ldfe      asin_coeff_P4  =    [ASIN_Addr2],16
     frsqrta.s1     asin_y0,p0     =    asin_t
} 
;;
 
{     .mfi 
     ldfe      asin_coeff_P16 =    [ASIN_Addr1],16
     fcmp.gt.s1 p9,p0 = asin_abs_x,f1
     nop.i                      999
} 
{     .mfb 
     ldfe      asin_coeff_P15 =    [ASIN_Addr2],16
(p8) fma.d     f8 = f8,f1,f0
(p8) br.ret.spnt b0
}
;;

 
{     .mmf 
     ldfe      asin_coeff_P20 =    [ASIN_Addr1],16
     ldfe      asin_coeff_P19 =    [ASIN_Addr2],16
     fclass.m.unc p8,p0 = f8, 0x07	//@zero
} 
;;
 

{     .mfi 
     ldfe      asin_coeff_P9  =    [ASIN_Addr1],16
     fma.s1    asin_t4        =    asin_t2,asin_t2,f0
(p9) mov GR_Parameter_Tag = 61 
} 
{     .mfi 
     ldfe      asin_coeff_P8  =    [ASIN_Addr2],16
     fma.s1    asin_3by2      =    asin_1by2,f1,f1
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      asin_coeff_P2  =    [ASIN_Addr2],16
     fma.s1    asin_tx4       =    asin_tx2,asin_tx2,f0
     nop.i                      999
} 
{     .mfb 
     ldfe      asin_coeff_P3  =    [ASIN_Addr1],16
     fma.s1    asin_t3        =    asin_t,asin_t2,f0
(p8) br.ret.spnt b0
}
;;

 
{     .mfi 
     ldfe      asin_coeff_P13 =    [ASIN_Addr2],16
     fma.s1    asin_H0        =    asin_y0,asin_1by2,f0
     nop.i                      999
} 
{     .mfb 
     ldfe      asin_coeff_P14 =    [ASIN_Addr1],16
     fma.s1    asin_S0        =    asin_y0,asin_t,f0
(p9) br.cond.spnt  __libm_error_region
}
;;

 
{     .mfi 
     ldfe      asin_coeff_P11 =    [ASIN_Addr2],16
     fcmp.eq.s1 p6,p0 = asin_abs_x,f1
     nop.i                      999
} 
{     .mfi 
     ldfe      asin_coeff_P12 =    [ASIN_Addr1],16
     fma.s1    asin_tx3       =    asin_tx,asin_tx2,f0
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      asin_coeff_P10 =    [ASIN_Addr2],16
     fma.s1    asin_1poly_p6  =    asin_tx,asin_coeff_P7,asin_coeff_P6
     nop.i                      999
} 
{     .mfi 
     ldfe      asin_coeff_P1  =    [ASIN_Addr1],16
     fma.s1    asin_poly_p6   =    asin_t,asin_coeff_P7,asin_coeff_P6
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      asin_const_sqrt2by2 =    [ASIN_Addr2],16
     fma.s1    asin_5by2           =    asin_3by2,f1,f1
     nop.i                           999
} 
{     .mfi 
     ldfe      asin_coeff_P21 =    [ASIN_Addr1],16
     fma.s1    asin_11by4     =    asin_3by2,asin_3by2,asin_1by2
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      asin_const_piby2    =    [ASIN_Addr1],16
     fma.s1    asin_poly_p17       =    asin_t,asin_coeff_P18,asin_coeff_P17
     nop.i                           999
} 
{     .mfb 
     nop.m                 999
     fma.s1    asin_3by4 =    asin_3by2,asin_1by2,f0
(p6) br.cond.spnt  L(ASIN_ABS_1)      // Branch to short exit if |x|=1
}
;;

 
{     .mfi 
     addl ASIN_lnorm_sig = -0x1,r0  // Form significand 0xffffffffffffffff
     fma.s1    asin_poly_p15  =    asin_t,asin_coeff_P16,asin_coeff_P15
     nop.i                      999
} 
{     .mfi 
     addl ASIN_snorm_exp = 0x0c001,r0  // Form small exponent
     fnma.s1   asin_d    =    asin_S0,asin_H0,asin_1by2
     nop.i                 999;;
}

 
// Form the exponent and significand of a small number
{     .mfi 
     setf.sig asin_eps_sig = ASIN_lnorm_sig
     fma.s1    asin_poly_p19  =    asin_t,asin_coeff_P20,asin_coeff_P19
     nop.i                      999
} 
{     .mfi 
     setf.exp asin_eps_exp = ASIN_snorm_exp
     fma.s1    asin_poly_p4   =    asin_t,asin_coeff_P5,asin_coeff_P4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p17 =    asin_tx,asin_coeff_P18,asin_coeff_P17
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p8   =    asin_t,asin_coeff_P9,asin_coeff_P8
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fms.s1    asin_35by8     =    asin_5by2,asin_11by4,asin_5by2
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_63by8     =    asin_5by2,asin_11by4,f1
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p13  =    asin_t,asin_coeff_P14,asin_coeff_P13
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_18by4     =    asin_3by2,asin_5by2,asin_3by4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    asin_l1   =    asin_5by2,asin_d,asin_3by2
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_d2   =    asin_d,asin_d,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p15  =    asin_t2,asin_poly_p17,asin_poly_p15
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_T0   =    asin_d,asin_S0,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p19  =    asin_t2,asin_coeff_P21,asin_poly_p19
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p4   =    asin_t2,asin_poly_p6,asin_poly_p4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    asin_d1   =    asin_35by8,asin_d,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_231by16   =    asin_3by2,asin_35by8,asin_63by8
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p2   =    asin_t,asin_coeff_P3,asin_coeff_P2
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p8   =    asin_t2,asin_coeff_P10,asin_poly_p8
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p11  =    asin_t,asin_coeff_P12,asin_coeff_P11
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_e0   =    asin_d2,asin_l1,asin_d
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p15 =    asin_tx,asin_coeff_P16,asin_coeff_P15
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p0   =    asin_t,asin_coeff_P1,f1
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p19 =    asin_tx,asin_coeff_P20,asin_coeff_P19
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p4  =    asin_tx,asin_coeff_P5,asin_coeff_P4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p8  =    asin_tx,asin_coeff_P9,asin_coeff_P8
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_l2   =    asin_231by16,asin_d,asin_63by8
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    asin_d3   =    asin_d2,asin_d,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_T3   =    asin_d2,asin_T0,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_429by16   =    asin_18by4,asin_11by4,asin_231by16
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_S1   =    asin_e0,asin_S0,asin_S0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p4   =    asin_t4,asin_poly_p8,asin_poly_p4
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p15  =    asin_t4,asin_poly_p19,asin_poly_p15
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p0   =    asin_t2,asin_poly_p2,asin_poly_p0
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p11  =    asin_t2,asin_poly_p13,asin_poly_p11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    asin_t8   =    asin_t4,asin_t4,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_e1   =    asin_d2,asin_l2,asin_d1
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p4  =    asin_tx2,asin_1poly_p6,asin_1poly_p4
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p15 =    asin_tx2,asin_1poly_p17,asin_1poly_p15
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p8  =    asin_tx2,asin_coeff_P10,asin_1poly_p8
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p19 =    asin_tx2,asin_coeff_P21,asin_1poly_p19
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p2  =    asin_tx,asin_coeff_P3,asin_coeff_P2
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p13 =    asin_tx,asin_coeff_P14,asin_coeff_P13
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p0  =    asin_tx,asin_coeff_P1,f1
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p11 =    asin_tx,asin_coeff_P12,asin_coeff_P11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    asin_l3   =    asin_429by16,asin_d,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_z    =    asin_e1,asin_T3,asin_S1
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p11  =    asin_t4,asin_poly_p15,asin_poly_p11
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_T6   =    asin_T3,asin_d3,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    asin_t11  =    asin_t8,asin_t3,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_poly_p0   =    asin_t4,asin_poly_p4,asin_poly_p0
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p4  =    asin_tx4,asin_1poly_p8,asin_1poly_p4
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p15 =    asin_tx4,asin_1poly_p19,asin_1poly_p15
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p0  =    asin_tx2,asin_1poly_p2,asin_1poly_p0
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p11 =    asin_tx2,asin_1poly_p13,asin_1poly_p11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                                                         999
//     fcmp.le.s1     asin_pred_LEsqrt2by2,asin_pred_GTsqrt2by2    =    asin_abs_x,asin_const_sqrt2by2
     fcmp.le.s1     p7,p8    =    asin_abs_x,asin_const_sqrt2by2
     nop.i                                                         999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_tx8  =    asin_tx4,asin_tx4,f0
     nop.i                 999;;
}

 
// Form a small number to force inexact flag for small args 
{     .mfi 
     nop.m                 999
     fmerge.se asin_eps = asin_eps_exp,asin_eps_sig
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    asin_z    =    asin_l3,asin_T6,asin_z
     nop.i                 999;;
} 
 
{     .mfi 
     nop.m                      999
     fma.s1    asin_series_t  =    asin_t11,asin_poly_p11,asin_poly_p0
     nop.i                      999;;
} 
 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p0  =    asin_tx4,asin_1poly_p4,asin_1poly_p0
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    asin_1poly_p11 =    asin_tx4,asin_1poly_p15,asin_1poly_p11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    asin_tx11 =    asin_tx8,asin_tx3,f0
     nop.i                 999;;
} 
 
{     .mfi 
                         nop.m                 999
//(asin_pred_GTsqrt2by2)   fnma.s1      answer2   =    asin_z,asin_series_t,asin_const_piby2
(p8)   fnma.s1      answer2   =    asin_z,asin_series_t,asin_const_piby2
                         nop.i                 999;;
} 
 
{     .mfi 
     nop.m                      999
     fma.s1    asin_series_tx =    asin_tx11,asin_1poly_p11,asin_1poly_p0
     nop.i                      999;;
} 
 
{     .mfi 
                         nop.m                 999
//(asin_pred_GTsqrt2by2)   fma.d     f8   =    asin_sgn_x,answer2,f0
(p8)   fma.d     f8   =    asin_sgn_x,answer2,f0
                         nop.i                 999;;
} 
 
// asin_eps is added only to force inexact and possibly underflow flag 
// in case asin_series_tx is zero
//
{     .mfi 
                         nop.m                 999
(p7)   fma.d     asin_eps   =    f8,asin_series_tx,asin_eps
                         nop.i                 999
} 
{     .mfb 
                         nop.m                 999
//(asin_pred_LEsqrt2by2)   fma.d     f8   =    f8,asin_series_tx,f0
(p7)   fma.d     f8   =    f8,asin_series_tx,f0
       br.ret.sptk b0
} 
;;


L(ASIN_ABS_1):
// Here for short exit if |x|=1
{     .mfb 
     nop.m                      999
     fma.d    f8 =    asin_sgn_x,asin_const_piby2,f0
     br.ret.sptk b0
} 
;;


.endp asin
ASM_SIZE_DIRECTIVE(asin)

.proc __libm_error_region
__libm_error_region:
.prologue
{ .mfi
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
                nop.f 999
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
        stfs [GR_Parameter_Y] = f1,16         // Store Parameter 2 on stack
        add GR_Parameter_X = 16,sp              // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                       // Save b0
};;

.body
        frcpa.s0 f9,p0 = f0,f0
;;

{ .mib
        stfd [GR_Parameter_X] = f8            // Store Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y
        nop.b 0                                 // Parameter 3 address
}
{ .mib
        stfd [GR_Parameter_Y] = f9,-16           // Store Parameter 3 on stack
        adds r32 = 48,sp
        br.call.sptk b0=__libm_error_support#   // Call error handling function
};;
{ .mmi
        ldfd  f8 = [r32]       // Get return result off stack
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

.type   __libm_error_support,@function
.global __libm_error_support
