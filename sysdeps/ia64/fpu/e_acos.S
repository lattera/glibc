.file "acos.s"

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
// 8/30/00  Avoided bank conflicts on loads, shortened |x|=1 and x=0 paths,
//          fixed mfb split issue stalls.

// Description
//=========================================
// The acos function computes the principle value of the arc sine of x.
// A doman error occurs for arguments not in the range [-1,+1].

// The acos function returns the arc cosine in the range [0, +pi] radians.
// acos(1) returns +0, acos(-1) returns pi, acos(0) returns pi/2.
// acos(x) returns a Nan and raises the invalid exception for |x| >1

// The acos function is just like asin except that pi/2 is added at the end.

//
// Assembly macros
//=========================================

#include "libm_support.h"

// predicate registers
//acos_pred_LEsqrt2by2            = p7
//acos_pred_GTsqrt2by2            = p8

// integer registers
ASIN_Addr1                      = r33
ASIN_Addr2                      = r34
ASIN_FFFE                       = r35

GR_SAVE_B0                      = r36
GR_SAVE_PFS                     = r37
GR_SAVE_GP                      = r38

GR_Parameter_X                  = r39
GR_Parameter_Y                  = r40
GR_Parameter_RESULT             = r41
GR_Parameter_Tag                = r42

// floating point registers
acos_coeff_P1                   = f32
acos_coeff_P2                   = f33
acos_coeff_P3                   = f34
acos_coeff_P4                   = f35

acos_coeff_P5                   = f36
acos_coeff_P6                   = f37
acos_coeff_P7                   = f38
acos_coeff_P8                   = f39
acos_coeff_P9                   = f40

acos_coeff_P10                  = f41
acos_coeff_P11                  = f42
acos_coeff_P12                  = f43
acos_coeff_P13                  = f44
acos_coeff_P14                  = f45

acos_coeff_P15                  = f46
acos_coeff_P16                  = f47
acos_coeff_P17                  = f48
acos_coeff_P18                  = f49
acos_coeff_P19                  = f50

acos_coeff_P20                  = f51
acos_coeff_P21                  = f52
acos_const_sqrt2by2             = f53
acos_const_piby2                = f54
acos_abs_x                      = f55

acos_tx                         = f56
acos_tx2                        = f57
acos_tx3                        = f58
acos_tx4                        = f59
acos_tx8                        = f60

acos_tx11                       = f61
acos_1poly_p8                   = f62
acos_1poly_p19                  = f63
acos_1poly_p4                   = f64
acos_1poly_p15                  = f65

acos_1poly_p6                   = f66
acos_1poly_p17                  = f67
acos_1poly_p0                   = f68
acos_1poly_p11                  = f69
acos_1poly_p2                   = f70

acos_1poly_p13                  = f71
acos_series_tx                  = f72
acos_t                          = f73
acos_t2                         = f74
acos_t3                         = f75

acos_t4                         = f76
acos_t8                         = f77
acos_t11                        = f78
acos_poly_p8                    = f79
acos_poly_p19                   = f80

acos_poly_p4                    = f81
acos_poly_p15                   = f82
acos_poly_p6                    = f83
acos_poly_p17                   = f84
acos_poly_p0                    = f85

acos_poly_p11                   = f86
acos_poly_p2                    = f87
acos_poly_p13                   = f88
acos_series_t                   = f89
acos_1by2                       = f90

acos_3by2                       = f91
acos_5by2                       = f92
acos_11by4                      = f93
acos_35by8                      = f94
acos_63by8                      = f95

acos_231by16                    = f96 
acos_y0                         = f97 
acos_H0                         = f98 
acos_S0                         = f99 
acos_d                          = f100

acos_l1                         = f101
acos_d2                         = f102
acos_T0                         = f103
acos_d1                         = f104
acos_e0                         = f105

acos_l2                         = f106
acos_d3                         = f107
acos_T3                         = f108
acos_S1                         = f109
acos_e1                         = f110

acos_z                          = f111
answer2                         = f112
acos_sgn_x                      = f113
acos_429by16                    = f114
acos_18by4                      = f115

acos_3by4                       = f116
acos_l3                         = f117
acos_T6                         = f118
acos_const_add                  = f119

// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

acos_coeff_1_table:
ASM_TYPE_DIRECTIVE(acos_coeff_1_table,@object)
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
data8 0x0000000000000000  , 0x00000000 // pad to avoid bank conflicts
ASM_SIZE_DIRECTIVE(acos_coeff_1_table)


acos_coeff_2_table:
ASM_TYPE_DIRECTIVE(acos_coeff_2_table,@object)
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
ASM_SIZE_DIRECTIVE(acos_coeff_2_table)


.align 32
.global acos
ASM_TYPE_DIRECTIVE(acos,@function)

.section .text
.proc  acos
.align 32


acos:
 
{     .mfi 
     alloc      r32               = ar.pfs,1,6,4,0
     fma.s1    acos_tx        =    f8,f8,f0
     addl      ASIN_Addr2     =    @ltoff(acos_coeff_2_table),gp
} 
{     .mfi 
     mov       ASIN_FFFE      =    0xFFFE
     fnma.s1   acos_t         =    f8,f8,f1
     addl      ASIN_Addr1     =    @ltoff(acos_coeff_1_table),gp
}
;;

 
{     .mfi 
     setf.exp       acos_1by2      =    ASIN_FFFE
     fmerge.s       acos_abs_x     =    f1,f8
     nop.i          999              ;;
} 
 

{     .mmf 
     ld8       ASIN_Addr1     =    [ASIN_Addr1]
     ld8       ASIN_Addr2     =    [ASIN_Addr2]
     fmerge.s  acos_sgn_x     =    f8,f1
} 
;;


{     .mfi 
     nop.m                      999
     fcmp.lt.s1  p11,p12  = f8, f0
     nop.i          999              ;;
} 
 
 
{     .mfi 
     ldfe      acos_coeff_P7  =    [ASIN_Addr1],16
     fma.s1    acos_tx2       =    acos_tx,acos_tx,f0
     nop.i                      999
} 
{     .mfi 
     ldfe      acos_coeff_P6  =    [ASIN_Addr2],16
     fma.s1    acos_t2        =    acos_t,acos_t,f0
     nop.i                      999;;
}

 
{     .mmf 
     ldfe      acos_coeff_P18 =    [ASIN_Addr1],16
     ldfe      acos_coeff_P17 =    [ASIN_Addr2],16
     fclass.m.unc p8,p0  = f8, 0xc3	//@qnan |@snan
} 
;;

 
{     .mmf 
     ldfe      acos_coeff_P5  =    [ASIN_Addr1],16
     ldfe      acos_coeff_P4  =    [ASIN_Addr2],16
     frsqrta.s1     acos_y0,p0     =    acos_t
} 
;;

 
{     .mfi 
     ldfe      acos_coeff_P16 =    [ASIN_Addr1],16
     fcmp.gt.s1 p9,p0 = acos_abs_x,f1
     nop.i                      999
} 
{     .mfb 
     ldfe      acos_coeff_P15 =    [ASIN_Addr2],16
(p8) fma.d     f8 = f8,f1,f0
(p8) br.ret.spnt b0
}
;;

 
{     .mmf 
     ldfe      acos_coeff_P20 =    [ASIN_Addr1],16
     ldfe      acos_coeff_P19 =    [ASIN_Addr2],16
     fclass.m.unc p10,p0 = f8, 0x07	//@zero
} 
;;

 
{     .mfi 
     ldfe      acos_coeff_P9  =    [ASIN_Addr1],16
     fma.s1    acos_t4        =    acos_t2,acos_t2,f0
(p9) mov GR_Parameter_Tag = 58 
} 
{     .mfi 
     ldfe      acos_coeff_P8  =    [ASIN_Addr2],16
     fma.s1    acos_3by2      =    acos_1by2,f1,f1
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      acos_coeff_P2  =    [ASIN_Addr2],16
     fma.s1    acos_tx4       =    acos_tx2,acos_tx2,f0
     nop.i 999
} 
{     .mfb 
     ldfe      acos_coeff_P3  =    [ASIN_Addr1],16
     fma.s1    acos_t3        =    acos_t,acos_t2,f0
(p9) br.cond.spnt  __libm_error_region
}
;;

 
{     .mfi 
     ldfe      acos_coeff_P13 =    [ASIN_Addr2],16
     fma.s1    acos_H0        =    acos_y0,acos_1by2,f0
     nop.i                      999
} 
{     .mfi 
     ldfe      acos_coeff_P14 =    [ASIN_Addr1],16
     fma.s1    acos_S0        =    acos_y0,acos_t,f0
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      acos_coeff_P11 =    [ASIN_Addr2],16
     fcmp.eq.s1  p6,p0  = acos_abs_x, f1
     nop.i                      999
} 
{     .mfi 
     ldfe      acos_coeff_P12 =    [ASIN_Addr1],16
     fma.s1    acos_tx3       =    acos_tx,acos_tx2,f0
     nop.i 999
}
;;

 
{     .mfi 
     ldfe      acos_coeff_P10 =    [ASIN_Addr2],16
     fma.s1    acos_1poly_p6  =    acos_tx,acos_coeff_P7,acos_coeff_P6
     nop.i                      999
} 
{     .mfi 
     ldfe      acos_coeff_P1  =    [ASIN_Addr1],16
     fma.s1    acos_poly_p6   =    acos_t,acos_coeff_P7,acos_coeff_P6
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      acos_const_sqrt2by2 =    [ASIN_Addr2],16
     fma.s1    acos_5by2           =    acos_3by2,f1,f1
     nop.i                           999
} 
{     .mfi 
     ldfe      acos_coeff_P21 =    [ASIN_Addr1],16
     fma.s1    acos_11by4     =    acos_3by2,acos_3by2,acos_1by2
     nop.i                      999;;
}

 
{     .mfi 
     ldfe      acos_const_piby2    =    [ASIN_Addr1],16
     fma.s1    acos_poly_p17       =    acos_t,acos_coeff_P18,acos_coeff_P17
     nop.i                      999
} 
{     .mfb 
     nop.m                 999
     fma.s1    acos_3by4 =    acos_3by2,acos_1by2,f0
(p10) br.cond.spnt  L(ACOS_ZERO)    // Branch to short path if x=0
}
;;

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p15  =    acos_t,acos_coeff_P16,acos_coeff_P15
     nop.i                      999
} 
{     .mfb 
     nop.m                 999
     fnma.s1   acos_d    =    acos_S0,acos_H0,acos_1by2
(p6) br.cond.spnt  L(ACOS_ABS_ONE)    // Branch to short path if |x|=1
}
;;

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p19  =    acos_t,acos_coeff_P20,acos_coeff_P19
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p4   =    acos_t,acos_coeff_P5,acos_coeff_P4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p17 =    acos_tx,acos_coeff_P18,acos_coeff_P17
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p8   =    acos_t,acos_coeff_P9,acos_coeff_P8
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fms.s1    acos_35by8     =    acos_5by2,acos_11by4,acos_5by2
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_63by8     =    acos_5by2,acos_11by4,f1
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p13  =    acos_t,acos_coeff_P14,acos_coeff_P13
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_18by4     =    acos_3by2,acos_5by2,acos_3by4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_l1   =    acos_5by2,acos_d,acos_3by2
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_d2   =    acos_d,acos_d,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p15  =    acos_t2,acos_poly_p17,acos_poly_p15
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_T0   =    acos_d,acos_S0,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p19  =    acos_t2,acos_coeff_P21,acos_poly_p19
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p4   =    acos_t2,acos_poly_p6,acos_poly_p4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_d1   =    acos_35by8,acos_d,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_231by16   =    acos_3by2,acos_35by8,acos_63by8
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p2   =    acos_t,acos_coeff_P3,acos_coeff_P2
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p8   =    acos_t2,acos_coeff_P10,acos_poly_p8
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p11  =    acos_t,acos_coeff_P12,acos_coeff_P11
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_e0   =    acos_d2,acos_l1,acos_d
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p15 =    acos_tx,acos_coeff_P16,acos_coeff_P15
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p0   =    acos_t,acos_coeff_P1,f1
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p19 =    acos_tx,acos_coeff_P20,acos_coeff_P19
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p4  =    acos_tx,acos_coeff_P5,acos_coeff_P4
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p8  =    acos_tx,acos_coeff_P9,acos_coeff_P8
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_l2   =    acos_231by16,acos_d,acos_63by8
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_d3   =    acos_d2,acos_d,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_T3   =    acos_d2,acos_T0,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_429by16   =    acos_18by4,acos_11by4,acos_231by16
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_S1   =    acos_e0,acos_S0,acos_S0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p4   =    acos_t4,acos_poly_p8,acos_poly_p4
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p15  =    acos_t4,acos_poly_p19,acos_poly_p15
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p0   =    acos_t2,acos_poly_p2,acos_poly_p0
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p11  =    acos_t2,acos_poly_p13,acos_poly_p11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_t8   =    acos_t4,acos_t4,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_e1   =    acos_d2,acos_l2,acos_d1
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p4  =    acos_tx2,acos_1poly_p6,acos_1poly_p4
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p15 =    acos_tx2,acos_1poly_p17,acos_1poly_p15
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p8  =    acos_tx2,acos_coeff_P10,acos_1poly_p8
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p19 =    acos_tx2,acos_coeff_P21,acos_1poly_p19
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p2  =    acos_tx,acos_coeff_P3,acos_coeff_P2
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p13 =    acos_tx,acos_coeff_P14,acos_coeff_P13
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p0  =    acos_tx,acos_coeff_P1,f1
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p11 =    acos_tx,acos_coeff_P12,acos_coeff_P11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_l3   =    acos_429by16,acos_d,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_z    =    acos_e1,acos_T3,acos_S1
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p11  =    acos_t4,acos_poly_p15,acos_poly_p11
     nop.i                      999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_T6   =    acos_T3,acos_d3,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_t11  =    acos_t8,acos_t3,f0
     nop.i                 999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_poly_p0   =    acos_t4,acos_poly_p4,acos_poly_p0
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p4  =    acos_tx4,acos_1poly_p8,acos_1poly_p4
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p15 =    acos_tx4,acos_1poly_p19,acos_1poly_p15
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p0  =    acos_tx2,acos_1poly_p2,acos_1poly_p0
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p11 =    acos_tx2,acos_1poly_p13,acos_1poly_p11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                                                         999
//     fcmp.le.s1     acos_pred_LEsqrt2by2,acos_pred_GTsqrt2by2    =    acos_abs_x,acos_const_sqrt2by2
     fcmp.le.s1     p7,p8    =    acos_abs_x,acos_const_sqrt2by2
     nop.i                                                         999
} 
{     .mfi 
     nop.m                 999
     fma.s1    acos_tx8  =    acos_tx4,acos_tx4,f0
     nop.i                 999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_z    =    acos_l3,acos_T6,acos_z
     nop.i                 999;;
} 
 
{     .mfi
     nop.m                      999
     fma.s1    acos_series_t  =    acos_t11,acos_poly_p11,acos_poly_p0
     nop.i                      999
}
{    .mfi
     nop.m 999
(p11) fma.s1 acos_const_add = acos_const_piby2, f1, acos_const_piby2
     nop.i 999
}
;;

{ .mfi
      nop.m 999
(p12) fma.s1 acos_const_add = f1,f0,f0
      nop.i 999
}
;;
 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p0  =    acos_tx4,acos_1poly_p4,acos_1poly_p0
     nop.i                      999
} 
{     .mfi 
     nop.m                      999
     fma.s1    acos_1poly_p11 =    acos_tx4,acos_1poly_p15,acos_1poly_p11
     nop.i                      999;;
}

 
{     .mfi 
     nop.m                 999
     fma.s1    acos_tx11 =    acos_tx8,acos_tx3,f0
     nop.i                 999;;
} 
 
{     .mfi 
                         nop.m                 999
//(acos_pred_GTsqrt2by2)   fnma.s1      answer2   =    acos_z,acos_series_t,acos_const_piby2
(p8)   fnma.s1      answer2   =    acos_z,acos_series_t,f0
                         nop.i                 999;;
} 
 
{     .mfi 
     nop.m                      999
     fma.s1    acos_series_tx =    acos_tx11,acos_1poly_p11,acos_1poly_p0
     nop.i                      999;;
} 
 
{     .mfi 
                         nop.m                 999
//(acos_pred_GTsqrt2by2)   fnma.d     f8   =    acos_sgn_x,answer2,acos_const_piby2
(p8)   fnma.d     f8   =    acos_sgn_x,answer2,acos_const_add
                         nop.i                 999;;
} 
 
{     .mfb 
                         nop.m                 999
//(acos_pred_LEsqrt2by2)   fnma.d     f8   =    f8,acos_series_tx,acos_const_piby2
(p7)   fnma.d     f8   =    f8,acos_series_tx,acos_const_piby2
     br.ret.sptk b0 ;;
} 


L(ACOS_ZERO):
// Here if x=0
{     .mfb 
      nop.m                 999
      fma.d    f8 =    acos_const_piby2,f1,f0
      br.ret.sptk b0 ;;
} 


L(ACOS_ABS_ONE):
.pred.rel "mutex",p11,p12
// Here if |x|=1
{     .mfi 
      nop.m                 999
(p11) fma.d    f8 =    acos_const_piby2,f1,acos_const_piby2 // acos(-1)=pi
      nop.i                 999
} 
{     .mfb 
      nop.m                 999
(p12) fma.d    f8 =    f1,f0,f0 // acos(1)=0
      br.ret.sptk b0 ;;
} 


.endp acos
ASM_SIZE_DIRECTIVE(acos)

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
