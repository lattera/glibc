.file "log.s"

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
// 6/16/00  Updated table to be rounded correctly
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 8/17/00  Improved speed of main path by 5 cycles
//          Shortened path for x=1.0
// 1/09/01  Improved speed, fixed flags for neg denormals
//
//
// API
//==============================================================
// double log(double)
// double log10(double)
//
// Overview of operation
//==============================================================
// Background
//
// Consider  x = 2^N 1.f1 f2 f3 f4...f63
// Log(x) = log(frcpa(x) x/frcpa(x))
//        = log(1/frcpa(x)) + log(frcpa(x) x)
//        = -log(frcpa(x)) + log(frcpa(x) x)
//
// frcpa(x)       = 2^-N frcpa((1.f1 f2 ... f63)
//
// -log(frcpa(x)) = -log(C) 
//                = -log(2^-N) - log(frcpa(1.f1 f2 ... f63))
//
// -log(frcpa(x)) = -log(C) 
//                = +Nlog2 - log(frcpa(1.f1 f2 ... f63))
//
// -log(frcpa(x)) = -log(C) 
//                = +Nlog2 + log(frcpa(1.f1 f2 ... f63))
//
// Log(x) = log(1/frcpa(x)) + log(frcpa(x) x)

// Log(x) =  +Nlog2 + log(1./frcpa(1.f1 f2 ... f63)) + log(frcpa(x) x)
// Log(x) =  +Nlog2 - log(/frcpa(1.f1 f2 ... f63))   + log(frcpa(x) x)
// Log(x) =  +Nlog2 + T                              + log(frcpa(x) x)
//
// Log(x) =  +Nlog2 + T                     + log(C x)
//
// Cx = 1 + r
//
// Log(x) =  +Nlog2 + T  + log(1+r)
// Log(x) =  +Nlog2 + T  + Series( r - r^2/2 + r^3/3 - r^4/4 ....)
//
// 1.f1 f2 ... f8 has 256 entries.
// They are 1 + k/2^8, k = 0 ... 255
// These 256 values are the table entries.
//
// Implementation
//===============
// CASE 1:  |x-1| >= 2^-6
// C = frcpa(x)
// r = C * x - 1
//
// Form rseries = r + P1*r^2 + P2*r^3 + P3*r^4 + P4*r^5 + P5*r^6
//
// x = f * 2*n where f is 1.f_1f_2f_3....f_63
// Nfloat = float(n)  where n is the true unbiased exponent
// pre-index = f_1f_2....f_8
// index = pre_index * 16
// get the dxt table entry at index + offset = T
//
// result = (T + Nfloat * log(2)) + rseries
//
// The T table is calculated as follows
// Form x_k = 1 + k/2^8 where k goes from 0... 255
//      y_k = frcpa(x_k)
//      log(1/y_k)  in quad and round to double-extended

// CASE 2:  |x-1| < 2^-6
// w = x - 1
//
// Form wseries = w + Q1*w^2 + Q2*w^3 + ... + Q7*w^8 + Q8*w^9
//
// result = wseries

// Special values 
//==============================================================


// log(+0)    = -inf
// log(-0)    = -inf

// log(+qnan) = +qnan 
// log(-qnan) = -qnan 
// log(+snan) = +qnan 
// log(-snan) = -qnan 

// log(-n)    = QNAN Indefinite
// log(-inf)  = QNAN Indefinite 

// log(+inf)  = +inf

// Registers used
//==============================================================
// Floating Point registers used: 
// f8, input
// f9 -> f15,  f32 -> f68

// General registers used:  
// r32 -> r51

// Predicate registers used:
// p6 -> p15

// p8 log base e
// p6 log base e special
// p9 used in the frcpa
// p13 log base e large W
// p14 log base e small w

// p7 log base 10
// p10 log base 10 large W
// p11 log base 10 small w
// p12 log base 10 special

#include "libm_support.h"

// Assembly macros
//==============================================================

log_int_Nfloat   = f9 
log_Nfloat       = f10 

log_P5           = f11 
log_P4           = f12 
log_P3           = f13 
log_P2           = f14 
log_half         = f15

log_log2         = f32 
log_T            = f33 

log_rp_p4        = f34 
log_rp_p32       = f35 
log_rp_p2        = f36 
log_w6           = f37
log_rp_p10       = f38
log_rcube        = f39
log_rsq          = f40 

log_T_plus_Nlog2 = f41 
log_w3           = f42

log_r            = f43
log_C            = f44

log_w            = f45
log_Q8           = f46
log_Q7           = f47
log_Q4           = f48 
log_Q3           = f49
log_Q6           = f50 
log_Q5           = f51
log_Q2           = f52
log_Q1           = f53 
log_P1           = f53 

log_rp_q7        = f54 
log_rp_q65       = f55
log_Qlo          = f56

log_rp_q3        = f57
log_rp_q21       = f58
log_Qhi          = f59

log_wsq          = f60
log_w4           = f61
log_Q            = f62

log_inv_ln10     = f63
log_log10_hi     = f64
log_log10_lo     = f65
log_rp_q10       = f66
log_NORM_f8      = f67
log_r2P_r        = f68 

// ===================================

log_GR_exp_17_ones               = r33
log_GR_exp_16_ones               = r34
log_GR_exp_f8                    = r35
log_GR_signexp_f8                = r36
log_GR_true_exp_f8               = r37
log_GR_significand_f8            = r38
log_GR_half_exp                  = r39
log_GR_index                     = r39
log_AD_1                         = r40
log_GR_signexp_w                 = r41
log_GR_fff9                      = r42
log_AD_2                         = r43
log_GR_exp_w                     = r44

GR_SAVE_B0                       = r45
GR_SAVE_GP                       = r46
GR_SAVE_PFS                      = r47

GR_Parameter_X                   = r48
GR_Parameter_Y                   = r49
GR_Parameter_RESULT              = r50
log_GR_tag                       = r51


// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

log_table_1:
ASM_TYPE_DIRECTIVE(log_table_1,@object)
data8 0xBFC5555DA7212371 // P5
data8 0x3FC999A19EEF5826 // P4
data8 0x3FBC756AC654273B // Q8
data8 0xBFC001A42489AB4D // Q7
data8 0x3FC99999999A169B // Q4
data8 0xBFD00000000019AC // Q3
ASM_SIZE_DIRECTIVE(log_table_1)
log_table_2:
ASM_TYPE_DIRECTIVE(log_table_2,@object)
data8 0xBFCFFFFFFFFEF009 // P3
data8 0x3FD555555554ECB2 // P2
data8 0x3FC2492479AA0DF8 // Q6
data8 0xBFC5555544986F52 // Q5
data8 0x3FD5555555555555 // Q2
data8 0xBFE0000000000000 // Q1, P1 = -0.5


data8 0xde5bd8a937287195, 0x00003ffd  // double-extended 1/ln(10)
data8 0xb17217f7d1cf79ac, 0x00003ffe  // log2
//      b17217f7d1cf79ab c9e3b39803f2f6a


data8 0x80200aaeac44ef38 , 0x00003ff6 //   log(1/frcpa(1+  0/2^-8))

data8 0xc09090a2c35aa070 , 0x00003ff7 //   log(1/frcpa(1+  1/2^-8))
data8 0xa0c94fcb41977c75 , 0x00003ff8 //   log(1/frcpa(1+  2/2^-8))
data8 0xe18b9c263af83301 , 0x00003ff8 //   log(1/frcpa(1+  3/2^-8))
data8 0x8d35c8d6399c30ea , 0x00003ff9 //   log(1/frcpa(1+  4/2^-8))
data8 0xadd4d2ecd601cbb8 , 0x00003ff9 //   log(1/frcpa(1+  5/2^-8))

data8 0xce95403a192f9f01 , 0x00003ff9 //   log(1/frcpa(1+  6/2^-8))
data8 0xeb59392cbcc01096 , 0x00003ff9 //   log(1/frcpa(1+  7/2^-8))
data8 0x862c7d0cefd54c5d , 0x00003ffa //   log(1/frcpa(1+  8/2^-8))
data8 0x94aa63c65e70d499 , 0x00003ffa //   log(1/frcpa(1+  9/2^-8))
data8 0xa54a696d4b62b382 , 0x00003ffa //   log(1/frcpa(1+ 10/2^-8))

data8 0xb3e4a796a5dac208 , 0x00003ffa //   log(1/frcpa(1+ 11/2^-8))
data8 0xc28c45b1878340a9 , 0x00003ffa //   log(1/frcpa(1+ 12/2^-8))
data8 0xd35c55f39d7a6235 , 0x00003ffa //   log(1/frcpa(1+ 13/2^-8))
data8 0xe220f037b954f1f5 , 0x00003ffa //   log(1/frcpa(1+ 14/2^-8))
data8 0xf0f3389b036834f3 , 0x00003ffa //   log(1/frcpa(1+ 15/2^-8))

data8 0xffd3488d5c980465 , 0x00003ffa //   log(1/frcpa(1+ 16/2^-8))
data8 0x87609ce2ed300490 , 0x00003ffb //   log(1/frcpa(1+ 17/2^-8))
data8 0x8ede9321e8c85927 , 0x00003ffb //   log(1/frcpa(1+ 18/2^-8))
data8 0x96639427f2f8e2f4 , 0x00003ffb //   log(1/frcpa(1+ 19/2^-8))
data8 0x9defad3e8f73217b , 0x00003ffb //   log(1/frcpa(1+ 20/2^-8))

data8 0xa582ebd50097029c , 0x00003ffb //   log(1/frcpa(1+ 21/2^-8))
data8 0xac06dbe75ab80fee , 0x00003ffb //   log(1/frcpa(1+ 22/2^-8))
data8 0xb3a78449b2d3ccca , 0x00003ffb //   log(1/frcpa(1+ 23/2^-8))
data8 0xbb4f79635ab46bb2 , 0x00003ffb //   log(1/frcpa(1+ 24/2^-8))
data8 0xc2fec93a83523f3f , 0x00003ffb //   log(1/frcpa(1+ 25/2^-8))

data8 0xc99af2eaca4c4571 , 0x00003ffb //   log(1/frcpa(1+ 26/2^-8))
data8 0xd1581106472fa653 , 0x00003ffb //   log(1/frcpa(1+ 27/2^-8))
data8 0xd8002560d4355f2e , 0x00003ffb //   log(1/frcpa(1+ 28/2^-8))
data8 0xdfcb43b4fe508632 , 0x00003ffb //   log(1/frcpa(1+ 29/2^-8))
data8 0xe67f6dff709d4119 , 0x00003ffb //   log(1/frcpa(1+ 30/2^-8))

data8 0xed393b1c22351280 , 0x00003ffb //   log(1/frcpa(1+ 31/2^-8))
data8 0xf5192bff087bcc35 , 0x00003ffb //   log(1/frcpa(1+ 32/2^-8))
data8 0xfbdf4ff6dfef2fa3 , 0x00003ffb //   log(1/frcpa(1+ 33/2^-8))
data8 0x81559a97f92f9cc7 , 0x00003ffc //   log(1/frcpa(1+ 34/2^-8))
data8 0x84be72bce90266e8 , 0x00003ffc //   log(1/frcpa(1+ 35/2^-8))

data8 0x88bc74113f23def2 , 0x00003ffc //   log(1/frcpa(1+ 36/2^-8))
data8 0x8c2ba3edf6799d11 , 0x00003ffc //   log(1/frcpa(1+ 37/2^-8))
data8 0x8f9dc92f92ea08b1 , 0x00003ffc //   log(1/frcpa(1+ 38/2^-8))
data8 0x9312e8f36efab5a7 , 0x00003ffc //   log(1/frcpa(1+ 39/2^-8))
data8 0x968b08643409ceb6 , 0x00003ffc //   log(1/frcpa(1+ 40/2^-8))

data8 0x9a062cba08a1708c , 0x00003ffc //   log(1/frcpa(1+ 41/2^-8))
data8 0x9d845b3abf95485c , 0x00003ffc //   log(1/frcpa(1+ 42/2^-8))
data8 0xa06fd841bc001bb4 , 0x00003ffc //   log(1/frcpa(1+ 43/2^-8))
data8 0xa3f3a74652fbe0db , 0x00003ffc //   log(1/frcpa(1+ 44/2^-8))
data8 0xa77a8fb2336f20f5 , 0x00003ffc //   log(1/frcpa(1+ 45/2^-8))

data8 0xab0497015d28b0a0 , 0x00003ffc //   log(1/frcpa(1+ 46/2^-8))
data8 0xae91c2be6ba6a615 , 0x00003ffc //   log(1/frcpa(1+ 47/2^-8))
data8 0xb189d1b99aebb20b , 0x00003ffc //   log(1/frcpa(1+ 48/2^-8))
data8 0xb51cced5de9c1b2c , 0x00003ffc //   log(1/frcpa(1+ 49/2^-8))
data8 0xb819bee9e720d42f , 0x00003ffc //   log(1/frcpa(1+ 50/2^-8))

data8 0xbbb2a0947b093a5d , 0x00003ffc //   log(1/frcpa(1+ 51/2^-8))
data8 0xbf4ec1505811684a , 0x00003ffc //   log(1/frcpa(1+ 52/2^-8))
data8 0xc2535bacfa8975ff , 0x00003ffc //   log(1/frcpa(1+ 53/2^-8))
data8 0xc55a3eafad187eb8 , 0x00003ffc //   log(1/frcpa(1+ 54/2^-8))
data8 0xc8ff2484b2c0da74 , 0x00003ffc //   log(1/frcpa(1+ 55/2^-8))

data8 0xcc0b1a008d53ab76 , 0x00003ffc //   log(1/frcpa(1+ 56/2^-8))
data8 0xcfb6203844b3209b , 0x00003ffc //   log(1/frcpa(1+ 57/2^-8))
data8 0xd2c73949a47a19f5 , 0x00003ffc //   log(1/frcpa(1+ 58/2^-8))
data8 0xd5daae18b49d6695 , 0x00003ffc //   log(1/frcpa(1+ 59/2^-8))
data8 0xd8f08248cf7e8019 , 0x00003ffc //   log(1/frcpa(1+ 60/2^-8))

data8 0xdca7749f1b3e540e , 0x00003ffc //   log(1/frcpa(1+ 61/2^-8))
data8 0xdfc28e033aaaf7c7 , 0x00003ffc //   log(1/frcpa(1+ 62/2^-8))
data8 0xe2e012a5f91d2f55 , 0x00003ffc //   log(1/frcpa(1+ 63/2^-8))
data8 0xe600064ed9e292a8 , 0x00003ffc //   log(1/frcpa(1+ 64/2^-8))
data8 0xe9226cce42b39f60 , 0x00003ffc //   log(1/frcpa(1+ 65/2^-8))

data8 0xec4749fd97a28360 , 0x00003ffc //   log(1/frcpa(1+ 66/2^-8))
data8 0xef6ea1bf57780495 , 0x00003ffc //   log(1/frcpa(1+ 67/2^-8))
data8 0xf29877ff38809091 , 0x00003ffc //   log(1/frcpa(1+ 68/2^-8))
data8 0xf5c4d0b245cb89be , 0x00003ffc //   log(1/frcpa(1+ 69/2^-8))
data8 0xf8f3afd6fcdef3aa , 0x00003ffc //   log(1/frcpa(1+ 70/2^-8))

data8 0xfc2519756be1abc7 , 0x00003ffc //   log(1/frcpa(1+ 71/2^-8))
data8 0xff59119f503e6832 , 0x00003ffc //   log(1/frcpa(1+ 72/2^-8))
data8 0x8147ce381ae0e146 , 0x00003ffd //   log(1/frcpa(1+ 73/2^-8))
data8 0x82e45f06cb1ad0f2 , 0x00003ffd //   log(1/frcpa(1+ 74/2^-8))
data8 0x842f5c7c573cbaa2 , 0x00003ffd //   log(1/frcpa(1+ 75/2^-8))

data8 0x85ce471968c8893a , 0x00003ffd //   log(1/frcpa(1+ 76/2^-8))
data8 0x876e8305bc04066d , 0x00003ffd //   log(1/frcpa(1+ 77/2^-8))
data8 0x891012678031fbb3 , 0x00003ffd //   log(1/frcpa(1+ 78/2^-8))
data8 0x8a5f1493d766a05f , 0x00003ffd //   log(1/frcpa(1+ 79/2^-8))
data8 0x8c030c778c56fa00 , 0x00003ffd //   log(1/frcpa(1+ 80/2^-8))

data8 0x8da85df17e31d9ae , 0x00003ffd //   log(1/frcpa(1+ 81/2^-8))
data8 0x8efa663e7921687e , 0x00003ffd //   log(1/frcpa(1+ 82/2^-8))
data8 0x90a22b6875c6a1f8 , 0x00003ffd //   log(1/frcpa(1+ 83/2^-8))
data8 0x91f62cc8f5d24837 , 0x00003ffd //   log(1/frcpa(1+ 84/2^-8))
data8 0x93a06cfc3857d980 , 0x00003ffd //   log(1/frcpa(1+ 85/2^-8))

data8 0x94f66d5e6fd01ced , 0x00003ffd //   log(1/frcpa(1+ 86/2^-8))
data8 0x96a330156e6772f2 , 0x00003ffd //   log(1/frcpa(1+ 87/2^-8))
data8 0x97fb3582754ea25b , 0x00003ffd //   log(1/frcpa(1+ 88/2^-8))
data8 0x99aa8259aad1bbf2 , 0x00003ffd //   log(1/frcpa(1+ 89/2^-8))
data8 0x9b0492f6227ae4a8 , 0x00003ffd //   log(1/frcpa(1+ 90/2^-8))

data8 0x9c5f8e199bf3a7a5 , 0x00003ffd //   log(1/frcpa(1+ 91/2^-8))
data8 0x9e1293b9998c1daa , 0x00003ffd //   log(1/frcpa(1+ 92/2^-8))
data8 0x9f6fa31e0b41f308 , 0x00003ffd //   log(1/frcpa(1+ 93/2^-8))
data8 0xa0cda11eaf46390e , 0x00003ffd //   log(1/frcpa(1+ 94/2^-8))
data8 0xa22c8f029cfa45aa , 0x00003ffd //   log(1/frcpa(1+ 95/2^-8))

data8 0xa3e48badb7856b34 , 0x00003ffd //   log(1/frcpa(1+ 96/2^-8))
data8 0xa5459a0aa95849f9 , 0x00003ffd //   log(1/frcpa(1+ 97/2^-8))
data8 0xa6a79c84480cfebd , 0x00003ffd //   log(1/frcpa(1+ 98/2^-8))
data8 0xa80a946d0fcb3eb2 , 0x00003ffd //   log(1/frcpa(1+ 99/2^-8))
data8 0xa96e831a3ea7b314 , 0x00003ffd //   log(1/frcpa(1+100/2^-8))

data8 0xaad369e3dc544e3b , 0x00003ffd //   log(1/frcpa(1+101/2^-8))
data8 0xac92e9588952c815 , 0x00003ffd //   log(1/frcpa(1+102/2^-8))
data8 0xadfa035aa1ed8fdc , 0x00003ffd //   log(1/frcpa(1+103/2^-8))
data8 0xaf6219eae1ad6e34 , 0x00003ffd //   log(1/frcpa(1+104/2^-8))
data8 0xb0cb2e6d8160f753 , 0x00003ffd //   log(1/frcpa(1+105/2^-8))

data8 0xb2354249ad950f72 , 0x00003ffd //   log(1/frcpa(1+106/2^-8))
data8 0xb3a056e98ef4a3b4 , 0x00003ffd //   log(1/frcpa(1+107/2^-8))
data8 0xb50c6dba52c6292a , 0x00003ffd //   log(1/frcpa(1+108/2^-8))
data8 0xb679882c33876165 , 0x00003ffd //   log(1/frcpa(1+109/2^-8))
data8 0xb78c07429785cedc , 0x00003ffd //   log(1/frcpa(1+110/2^-8))

data8 0xb8faeb8dc4a77d24 , 0x00003ffd //   log(1/frcpa(1+111/2^-8))
data8 0xba6ad77eb36ae0d6 , 0x00003ffd //   log(1/frcpa(1+112/2^-8))
data8 0xbbdbcc915e9bee50 , 0x00003ffd //   log(1/frcpa(1+113/2^-8))
data8 0xbd4dcc44f8cf12ef , 0x00003ffd //   log(1/frcpa(1+114/2^-8))
data8 0xbec0d81bf5b531fa , 0x00003ffd //   log(1/frcpa(1+115/2^-8))

data8 0xc034f19c139186f4 , 0x00003ffd //   log(1/frcpa(1+116/2^-8))
data8 0xc14cb69f7c5e55ab , 0x00003ffd //   log(1/frcpa(1+117/2^-8))
data8 0xc2c2abbb6e5fd56f , 0x00003ffd //   log(1/frcpa(1+118/2^-8))
data8 0xc439b2c193e6771e , 0x00003ffd //   log(1/frcpa(1+119/2^-8))
data8 0xc553acb9d5c67733 , 0x00003ffd //   log(1/frcpa(1+120/2^-8))

data8 0xc6cc96e441272441 , 0x00003ffd //   log(1/frcpa(1+121/2^-8))
data8 0xc8469753eca88c30 , 0x00003ffd //   log(1/frcpa(1+122/2^-8))
data8 0xc962cf3ce072b05c , 0x00003ffd //   log(1/frcpa(1+123/2^-8))
data8 0xcadeba8771f694aa , 0x00003ffd //   log(1/frcpa(1+124/2^-8))
data8 0xcc5bc08d1f72da94 , 0x00003ffd //   log(1/frcpa(1+125/2^-8))

data8 0xcd7a3f99ea035c29 , 0x00003ffd //   log(1/frcpa(1+126/2^-8))
data8 0xcef93860c8a53c35 , 0x00003ffd //   log(1/frcpa(1+127/2^-8))
data8 0xd0192f68a7ed23df , 0x00003ffd //   log(1/frcpa(1+128/2^-8))
data8 0xd19a201127d3c645 , 0x00003ffd //   log(1/frcpa(1+129/2^-8))
data8 0xd2bb92f4061c172c , 0x00003ffd //   log(1/frcpa(1+130/2^-8))

data8 0xd43e80b2ee8cc8fc , 0x00003ffd //   log(1/frcpa(1+131/2^-8))
data8 0xd56173601fc4ade4 , 0x00003ffd //   log(1/frcpa(1+132/2^-8))
data8 0xd6e6637efb54086f , 0x00003ffd //   log(1/frcpa(1+133/2^-8))
data8 0xd80ad9f58f3c8193 , 0x00003ffd //   log(1/frcpa(1+134/2^-8))
data8 0xd991d1d31aca41f8 , 0x00003ffd //   log(1/frcpa(1+135/2^-8))

data8 0xdab7d02231484a93 , 0x00003ffd //   log(1/frcpa(1+136/2^-8))
data8 0xdc40d532cde49a54 , 0x00003ffd //   log(1/frcpa(1+137/2^-8))
data8 0xdd685f79ed8b265e , 0x00003ffd //   log(1/frcpa(1+138/2^-8))
data8 0xde9094bbc0e17b1d , 0x00003ffd //   log(1/frcpa(1+139/2^-8))
data8 0xe01c91b78440c425 , 0x00003ffd //   log(1/frcpa(1+140/2^-8))

data8 0xe14658f26997e729 , 0x00003ffd //   log(1/frcpa(1+141/2^-8))
data8 0xe270cdc2391e0d23 , 0x00003ffd //   log(1/frcpa(1+142/2^-8))
data8 0xe3ffce3a2aa64922 , 0x00003ffd //   log(1/frcpa(1+143/2^-8))
data8 0xe52bdb274ed82887 , 0x00003ffd //   log(1/frcpa(1+144/2^-8))
data8 0xe6589852e75d7df6 , 0x00003ffd //   log(1/frcpa(1+145/2^-8))

data8 0xe786068c79937a7d , 0x00003ffd //   log(1/frcpa(1+146/2^-8))
data8 0xe91903adad100911 , 0x00003ffd //   log(1/frcpa(1+147/2^-8))
data8 0xea481236f7d35bb0 , 0x00003ffd //   log(1/frcpa(1+148/2^-8))
data8 0xeb77d48c692e6b14 , 0x00003ffd //   log(1/frcpa(1+149/2^-8))
data8 0xeca84b83d7297b87 , 0x00003ffd //   log(1/frcpa(1+150/2^-8))

data8 0xedd977f4962aa158 , 0x00003ffd //   log(1/frcpa(1+151/2^-8))
data8 0xef7179a22f257754 , 0x00003ffd //   log(1/frcpa(1+152/2^-8))
data8 0xf0a450d139366ca7 , 0x00003ffd //   log(1/frcpa(1+153/2^-8))
data8 0xf1d7e0524ff9ffdb , 0x00003ffd //   log(1/frcpa(1+154/2^-8))
data8 0xf30c29036a8b6cae , 0x00003ffd //   log(1/frcpa(1+155/2^-8))

data8 0xf4412bc411ea8d92 , 0x00003ffd //   log(1/frcpa(1+156/2^-8))
data8 0xf576e97564c8619d , 0x00003ffd //   log(1/frcpa(1+157/2^-8))
data8 0xf6ad62fa1b5f172f , 0x00003ffd //   log(1/frcpa(1+158/2^-8))
data8 0xf7e499368b55c542 , 0x00003ffd //   log(1/frcpa(1+159/2^-8))
data8 0xf91c8d10abaffe22 , 0x00003ffd //   log(1/frcpa(1+160/2^-8))

data8 0xfa553f7018c966f3 , 0x00003ffd //   log(1/frcpa(1+161/2^-8))
data8 0xfb8eb13e185d802c , 0x00003ffd //   log(1/frcpa(1+162/2^-8))
data8 0xfcc8e3659d9bcbed , 0x00003ffd //   log(1/frcpa(1+163/2^-8))
data8 0xfe03d6d34d487fd2 , 0x00003ffd //   log(1/frcpa(1+164/2^-8))
data8 0xff3f8c7581e9f0ae , 0x00003ffd //   log(1/frcpa(1+165/2^-8))

data8 0x803e029e280173ae , 0x00003ffe //   log(1/frcpa(1+166/2^-8))
data8 0x80dca10cc52d0757 , 0x00003ffe //   log(1/frcpa(1+167/2^-8))
data8 0x817ba200632755a1 , 0x00003ffe //   log(1/frcpa(1+168/2^-8))
data8 0x821b05f3b01d6774 , 0x00003ffe //   log(1/frcpa(1+169/2^-8))
data8 0x82bacd623ff19d06 , 0x00003ffe //   log(1/frcpa(1+170/2^-8))

data8 0x835af8c88e7a8f47 , 0x00003ffe //   log(1/frcpa(1+171/2^-8))
data8 0x83c5f8299e2b4091 , 0x00003ffe //   log(1/frcpa(1+172/2^-8))
data8 0x8466cb43f3d87300 , 0x00003ffe //   log(1/frcpa(1+173/2^-8))
data8 0x850803a67c80ca4b , 0x00003ffe //   log(1/frcpa(1+174/2^-8))
data8 0x85a9a1d11a23b461 , 0x00003ffe //   log(1/frcpa(1+175/2^-8))

data8 0x864ba644a18e6e05 , 0x00003ffe //   log(1/frcpa(1+176/2^-8))
data8 0x86ee1182dcc432f7 , 0x00003ffe //   log(1/frcpa(1+177/2^-8))
data8 0x875a925d7e48c316 , 0x00003ffe //   log(1/frcpa(1+178/2^-8))
data8 0x87fdaa109d23aef7 , 0x00003ffe //   log(1/frcpa(1+179/2^-8))
data8 0x88a129ed4becfaf2 , 0x00003ffe //   log(1/frcpa(1+180/2^-8))

data8 0x89451278ecd7f9cf , 0x00003ffe //   log(1/frcpa(1+181/2^-8))
data8 0x89b29295f8432617 , 0x00003ffe //   log(1/frcpa(1+182/2^-8))
data8 0x8a572ac5a5496882 , 0x00003ffe //   log(1/frcpa(1+183/2^-8))
data8 0x8afc2d0ce3b2dadf , 0x00003ffe //   log(1/frcpa(1+184/2^-8))
data8 0x8b6a69c608cfd3af , 0x00003ffe //   log(1/frcpa(1+185/2^-8))

data8 0x8c101e106e899a83 , 0x00003ffe //   log(1/frcpa(1+186/2^-8))
data8 0x8cb63de258f9d626 , 0x00003ffe //   log(1/frcpa(1+187/2^-8))
data8 0x8d2539c5bd19e2b1 , 0x00003ffe //   log(1/frcpa(1+188/2^-8))
data8 0x8dcc0e064b29e6f1 , 0x00003ffe //   log(1/frcpa(1+189/2^-8))
data8 0x8e734f45d88357ae , 0x00003ffe //   log(1/frcpa(1+190/2^-8))

data8 0x8ee30cef034a20db , 0x00003ffe //   log(1/frcpa(1+191/2^-8))
data8 0x8f8b0515686d1d06 , 0x00003ffe //   log(1/frcpa(1+192/2^-8))
data8 0x90336bba039bf32f , 0x00003ffe //   log(1/frcpa(1+193/2^-8))
data8 0x90a3edd23d1c9d58 , 0x00003ffe //   log(1/frcpa(1+194/2^-8))
data8 0x914d0de2f5d61b32 , 0x00003ffe //   log(1/frcpa(1+195/2^-8))

data8 0x91be0c20d28173b5 , 0x00003ffe //   log(1/frcpa(1+196/2^-8))
data8 0x9267e737c06cd34a , 0x00003ffe //   log(1/frcpa(1+197/2^-8))
data8 0x92d962ae6abb1237 , 0x00003ffe //   log(1/frcpa(1+198/2^-8))
data8 0x9383fa6afbe2074c , 0x00003ffe //   log(1/frcpa(1+199/2^-8))
data8 0x942f0421651c1c4e , 0x00003ffe //   log(1/frcpa(1+200/2^-8))

data8 0x94a14a3845bb985e , 0x00003ffe //   log(1/frcpa(1+201/2^-8))
data8 0x954d133857f861e7 , 0x00003ffe //   log(1/frcpa(1+202/2^-8))
data8 0x95bfd96468e604c4 , 0x00003ffe //   log(1/frcpa(1+203/2^-8))
data8 0x9632d31cafafa858 , 0x00003ffe //   log(1/frcpa(1+204/2^-8))
data8 0x96dfaabd86fa1647 , 0x00003ffe //   log(1/frcpa(1+205/2^-8))

data8 0x9753261fcbb2a594 , 0x00003ffe //   log(1/frcpa(1+206/2^-8))
data8 0x9800c11b426b996d , 0x00003ffe //   log(1/frcpa(1+207/2^-8))
data8 0x9874bf4d45ae663c , 0x00003ffe //   log(1/frcpa(1+208/2^-8))
data8 0x99231f5ee9a74f79 , 0x00003ffe //   log(1/frcpa(1+209/2^-8))
data8 0x9997a18a56bcad28 , 0x00003ffe //   log(1/frcpa(1+210/2^-8))

data8 0x9a46c873a3267e79 , 0x00003ffe //   log(1/frcpa(1+211/2^-8))
data8 0x9abbcfc621eb6cb6 , 0x00003ffe //   log(1/frcpa(1+212/2^-8))
data8 0x9b310cb0d354c990 , 0x00003ffe //   log(1/frcpa(1+213/2^-8))
data8 0x9be14cf9e1b3515c , 0x00003ffe //   log(1/frcpa(1+214/2^-8))
data8 0x9c5710b8cbb73a43 , 0x00003ffe //   log(1/frcpa(1+215/2^-8))

data8 0x9ccd0abd301f399c , 0x00003ffe //   log(1/frcpa(1+216/2^-8))
data8 0x9d7e67f3bdce8888 , 0x00003ffe //   log(1/frcpa(1+217/2^-8))
data8 0x9df4ea81a99daa01 , 0x00003ffe //   log(1/frcpa(1+218/2^-8))
data8 0x9e6ba405a54514ba , 0x00003ffe //   log(1/frcpa(1+219/2^-8))
data8 0x9f1e21c8c7bb62b3 , 0x00003ffe //   log(1/frcpa(1+220/2^-8))

data8 0x9f956593f6b6355c , 0x00003ffe //   log(1/frcpa(1+221/2^-8))
data8 0xa00ce1092e5498c3 , 0x00003ffe //   log(1/frcpa(1+222/2^-8))
data8 0xa0c08309c4b912c1 , 0x00003ffe //   log(1/frcpa(1+223/2^-8))
data8 0xa1388a8c6faa2afa , 0x00003ffe //   log(1/frcpa(1+224/2^-8))
data8 0xa1b0ca7095b5f985 , 0x00003ffe //   log(1/frcpa(1+225/2^-8))

data8 0xa22942eb47534a00 , 0x00003ffe //   log(1/frcpa(1+226/2^-8))
data8 0xa2de62326449d0a3 , 0x00003ffe //   log(1/frcpa(1+227/2^-8))
data8 0xa357690f88bfe345 , 0x00003ffe //   log(1/frcpa(1+228/2^-8))
data8 0xa3d0a93f45169a4b , 0x00003ffe //   log(1/frcpa(1+229/2^-8))
data8 0xa44a22f7ffe65f30 , 0x00003ffe //   log(1/frcpa(1+230/2^-8))

data8 0xa500c5e5b4c1aa36 , 0x00003ffe //   log(1/frcpa(1+231/2^-8))
data8 0xa57ad064eb2ebbc2 , 0x00003ffe //   log(1/frcpa(1+232/2^-8))
data8 0xa5f5152dedf4384e , 0x00003ffe //   log(1/frcpa(1+233/2^-8))
data8 0xa66f9478856233ec , 0x00003ffe //   log(1/frcpa(1+234/2^-8))
data8 0xa6ea4e7cca02c32e , 0x00003ffe //   log(1/frcpa(1+235/2^-8))

data8 0xa765437325341ccf , 0x00003ffe //   log(1/frcpa(1+236/2^-8))
data8 0xa81e21e6c75b4020 , 0x00003ffe //   log(1/frcpa(1+237/2^-8))
data8 0xa899ab333fe2b9ca , 0x00003ffe //   log(1/frcpa(1+238/2^-8))
data8 0xa9157039c51ebe71 , 0x00003ffe //   log(1/frcpa(1+239/2^-8))
data8 0xa991713433c2b999 , 0x00003ffe //   log(1/frcpa(1+240/2^-8))

data8 0xaa0dae5cbcc048b3 , 0x00003ffe //   log(1/frcpa(1+241/2^-8))
data8 0xaa8a27ede5eb13ad , 0x00003ffe //   log(1/frcpa(1+242/2^-8))
data8 0xab06de228a9e3499 , 0x00003ffe //   log(1/frcpa(1+243/2^-8))
data8 0xab83d135dc633301 , 0x00003ffe //   log(1/frcpa(1+244/2^-8))
data8 0xac3fb076adc7fe7a , 0x00003ffe //   log(1/frcpa(1+245/2^-8))

data8 0xacbd3cbbe47988f1 , 0x00003ffe //   log(1/frcpa(1+246/2^-8))
data8 0xad3b06b1a5dc57c3 , 0x00003ffe //   log(1/frcpa(1+247/2^-8))
data8 0xadb90e94af887717 , 0x00003ffe //   log(1/frcpa(1+248/2^-8))
data8 0xae3754a218f7c816 , 0x00003ffe //   log(1/frcpa(1+249/2^-8))
data8 0xaeb5d9175437afa2 , 0x00003ffe //   log(1/frcpa(1+250/2^-8))

data8 0xaf349c322e9c7cee , 0x00003ffe //   log(1/frcpa(1+251/2^-8))
data8 0xafb39e30d1768d1c , 0x00003ffe //   log(1/frcpa(1+252/2^-8))
data8 0xb032df51c2c93116 , 0x00003ffe //   log(1/frcpa(1+253/2^-8))
data8 0xb0b25fd3e6035ad9 , 0x00003ffe //   log(1/frcpa(1+254/2^-8))
data8 0xb1321ff67cba178c , 0x00003ffe //   log(1/frcpa(1+255/2^-8))
ASM_SIZE_DIRECTIVE(log_table_2)

   
.align 32
.global log#
.global log10#

// log10 has p7 true, p8 false
// log   has p8 true, p7 false

.section .text
.proc  log10#
.align 32

log10:
#ifdef _LIBC
.global __ieee754_log10
.type __ieee754_log10,@function
__ieee754_log10:
#endif
{ .mfi
     alloc     r32=ar.pfs,1,15,4,0                    
     frcpa.s1  log_C,p9 = f1,f8                 
     cmp.eq.unc     p7,p8         = r0, r0 
}
{ .mfb
     addl           log_AD_1   = @ltoff(log_table_1), gp
     fnorm.s1 log_NORM_f8 = f8 
     br.sptk        L(LOG_LOG10_X) 
}
;;

.endp log10
ASM_SIZE_DIRECTIVE(log10)
ASM_SIZE_DIRECTIVE(__ieee754_log10)


.section .text
.proc  log#
.align 32
log: 
#ifdef _LIBC
.global __ieee754_log
.type __ieee754_log,@function
__ieee754_log:
#endif

{ .mfi
     alloc     r32=ar.pfs,1,15,4,0                    
     frcpa.s1  log_C,p9 = f1,f8                 
     cmp.eq.unc     p8,p7         = r0, r0 
}
{ .mfi
     addl           log_AD_1   = @ltoff(log_table_1), gp
     fnorm.s1 log_NORM_f8 = f8 
     nop.i 999
}
;;

L(LOG_LOG10_X):

{ .mfi
     ld8 log_AD_1 = [log_AD_1]
     fclass.m.unc p15,p0 = f8, 0x0b            // Test for x=unorm
     mov        log_GR_fff9 = 0xfff9
}
{ .mfi
     mov       log_GR_half_exp = 0x0fffe
     fms.s1     log_w = f8,f1,f1              
     mov       log_GR_exp_17_ones = 0x1ffff
}
;;

{ .mmi
     getf.exp   log_GR_signexp_f8 = f8 // If x unorm then must recompute
     setf.exp   log_half = log_GR_half_exp  // Form 0.5 = -Q1
     nop.i 999
}
;;

{ .mmb
     adds log_AD_2 = 0x30, log_AD_1
     mov       log_GR_exp_16_ones = 0xffff
(p15) br.cond.spnt L(LOG_DENORM)     
}
;;

L(LOG_COMMON):
{.mfi
     ldfpd      log_P5,log_P4 = [log_AD_1],16           
     fclass.m.unc p6,p0 = f8, 0xc3             // Test for x=nan
     and        log_GR_exp_f8 = log_GR_signexp_f8, log_GR_exp_17_ones  
}
{.mfi
     ldfpd      log_P3,log_P2 = [log_AD_2],16           
     nop.f 999
     nop.i 999
}
;;

{ .mfi
     ldfpd      log_Q8,log_Q7 = [log_AD_1],16           
     fclass.m.unc p11,p0 = f8, 0x21            // Test for x=+inf
     sub       log_GR_true_exp_f8 = log_GR_exp_f8, log_GR_exp_16_ones 
}
{ .mfi
     ldfpd      log_Q6,log_Q5 = [log_AD_2],16           
     nop.f 999
     nop.i 999
}
;;


{ .mfi
     ldfpd      log_Q4,log_Q3 = [log_AD_1],16           
     fma.s1     log_wsq     = log_w, log_w, f0
     nop.i 999
}
{ .mfb
     ldfpd      log_Q2,log_Q1 = [log_AD_2],16           
(p6) fma.d.s0   f8 = f8,f1,f0      // quietize nan result if x=nan
(p6) br.ret.spnt b0                // Exit for x=nan
}
;;


{ .mfi
     setf.sig  log_int_Nfloat = log_GR_true_exp_f8
     fcmp.eq.s1 p10,p0 = log_NORM_f8, f1  // Test for x=+1.0
     nop.i 999
}
{ .mfb
     nop.m 999
     fms.s1     log_r = log_C,f8,f1
(p11) br.ret.spnt b0               // Exit for x=+inf
}
;;


{ .mmf
     getf.sig   log_GR_significand_f8 = log_NORM_f8 
     ldfe       log_inv_ln10 = [log_AD_2],16      
     fclass.m.unc p6,p0 = f8, 0x07        // Test for x=0
}
;;


{ .mfb
     nop.m 999
(p10) fmerge.s f8 = f0, f0
(p10) br.ret.spnt b0                // Exit for x=1.0
;;
}

{ .mfi
     getf.exp   log_GR_signexp_w = log_w
     fclass.m.unc p12,p0 = f8, 0x3a       // Test for x neg norm, unorm, inf
     shl        log_GR_index = log_GR_significand_f8,1            
}
;;

{ .mfi
     ldfe       log_log2 = [log_AD_2],16   
     fnma.s1    log_rp_q10 = log_half, log_wsq, log_w
     shr.u     log_GR_index = log_GR_index,56
}
{ .mfb
     nop.m 999
     fma.s1      log_w3      = log_wsq, log_w, f0
(p6) br.cond.spnt L(LOG_ZERO_NEG)      // Branch if x=0
;;
}
 

{ .mfi
     and log_GR_exp_w = log_GR_exp_17_ones, log_GR_signexp_w
     fma.s1      log_w4      = log_wsq, log_wsq, f0
     nop.i 999
}
{ .mfb
     shladd log_AD_2 = log_GR_index,4,log_AD_2
     fma.s1     log_rsq     = log_r, log_r, f0                   
(p12) br.cond.spnt L(LOG_ZERO_NEG)     // Branch if x<0
;;
}

{ .mfi
     ldfe       log_T = [log_AD_2]
     fma.s1    log_rp_p4   = log_P5, log_r, log_P4
     nop.i 999
}
{ .mfi
     nop.m 999
     fma.s1      log_rp_p32 = log_P3, log_r, log_P2
     nop.i 999
;;
}


{ .mfi
     nop.m 999
     fma.s1    log_rp_q7   = log_Q8, log_w, log_Q7
     nop.i 999
}
{ .mfi
     nop.m 999
     fma.s1    log_rp_q65  = log_Q6, log_w, log_Q5
     nop.i 999
;;
}

//    p13 <== large w log
//    p14 <== small w log
{ .mfi
(p8) cmp.ge.unc p13,p14 = log_GR_exp_w, log_GR_fff9
     fma.s1    log_rp_q3   = log_Q4, log_w, log_Q3
     nop.i 999
;;
}

//    p10 <== large w log10
//    p11 <== small w log10
{ .mfi
(p7) cmp.ge.unc p10,p11 = log_GR_exp_w, log_GR_fff9
     fcvt.xf   log_Nfloat = log_int_Nfloat
     nop.i 999
}

{ .mfi
     nop.m 999
     fma.s1    log_rp_q21  = log_Q2, log_w3, log_rp_q10
     nop.i 999 ;;
}

{ .mfi
     nop.m 999
     fma.s1    log_rcube   = log_rsq, log_r, f0
     nop.i 999
}
{ .mfi
     nop.m 999
     fma.s1    log_rp_p10   = log_rsq, log_P1, log_r
     nop.i 999
;;
}

{ .mfi
     nop.m 999
     fcmp.eq.s0 p6,p0 = f8,f0         // Sets flag on +denormal input
     nop.i 999
}
{ .mfi
     nop.m 999
     fma.s1     log_rp_p2   = log_rp_p4, log_rsq, log_rp_p32
     nop.i 999
;;
}


{ .mfi
     nop.m 999
     fma.s1        log_w6     = log_w3, log_w3, f0           
     nop.i 999 
}
{ .mfi
     nop.m 999
     fma.s1        log_Qlo     = log_rp_q7, log_wsq, log_rp_q65           
     nop.i 999 
}
;;

{ .mfi
     nop.m 999
     fma.s1        log_Qhi     = log_rp_q3, log_w4, log_rp_q21
     nop.i 999 ;;
}


{ .mfi
     nop.m 999
     fma.s1        log_T_plus_Nlog2 = log_Nfloat,log_log2, log_T    
     nop.i 999 ;;
}

{ .mfi
     nop.m 999
     fma.s1        log_r2P_r = log_rp_p2, log_rcube, log_rp_p10           
     nop.i 999 ;;
}


//    small w, log   <== p14
{ .mfi
     nop.m 999
(p14) fma.d        f8       = log_Qlo, log_w6, log_Qhi          
     nop.i 999
}
{ .mfi
     nop.m 999
     fma.s1        log_Q       = log_Qlo, log_w6, log_Qhi          
     nop.i 999 ;;
}


{ .mfi
     nop.m 999
(p10) fma.s1        log_log10_hi     = log_T_plus_Nlog2, log_inv_ln10,f0
     nop.i 999  ;;
}

//    large w, log   <== p13
.pred.rel "mutex",p13,p10
{ .mfi
      nop.m 999
(p13) fadd.d        f8              = log_T_plus_Nlog2, log_r2P_r 
      nop.i 999 
}
{ .mfi
      nop.m 999
(p10) fma.s1     log_log10_lo     = log_inv_ln10, log_r2P_r,f0
      nop.i 999  ;;
}


//    small w, log10 <== p11
{ .mfi
      nop.m 999
(p11) fma.d      f8 = log_inv_ln10,log_Q,f0                         
      nop.i 999 ;;
}

//    large w, log10 <== p10
{ .mfb
      nop.m 999
(p10) fma.d      f8                = log_log10_hi, f1, log_log10_lo 
      br.ret.sptk     b0 
;;
}

L(LOG_DENORM):
{ .mfb
     getf.exp   log_GR_signexp_f8 = log_NORM_f8 
     nop.f 999
     br.cond.sptk L(LOG_COMMON)
}
;;

L(LOG_ZERO_NEG): 

// qnan snan inf norm     unorm 0 -+
// 0    0    0   0        0     1 11      0x7
// 0    0    1   1        1     0 10      0x3a

// Save x (f8) in f10
{ .mfi
     nop.m 999
     fmerge.s f10 = f8,f8 
     nop.i 999  ;;
}

// p8 p9  means  ln(+-0)  = -inf
// p7 p10 means  log(+-0) = -inf

//    p13 means  ln(-)
//    p14 means  log(-)


{ .mfi
     nop.m 999
     fmerge.ns   f6 = f1,f1            // Form -1.0
     nop.i 999  ;;
}

// p9  means  ln(+-0)  = -inf
// p10 means  log(+-0) = -inf
// Log(+-0) = -inf 

{ .mfi
	nop.m 999
(p8)  fclass.m.unc  p9,p0 = f10, 0x07           
	nop.i 999
}
{ .mfi
	nop.m 999
(p7)  fclass.m.unc  p10,p0 = f10, 0x07           
	nop.i 999 ;;
}


// p13  ln(-)
// p14  log(-)

// Log(-inf, -normal, -unnormal) = QNAN indefinite
{ .mfi
	nop.m 999
(p8)  fclass.m.unc  p13,p0 = f10, 0x3a           
	nop.i 999 
}
{ .mfi
	nop.m 999
(p7)  fclass.m.unc  p14,p0 = f10, 0x3a           
	nop.i 999  ;;
}


.pred.rel "mutex",p9,p10
{ .mfi
(p9)     mov        log_GR_tag = 2       
(p9)    frcpa f8,p11 = f6,f0                   
            nop.i 999
}
{ .mfi
(p10)    mov        log_GR_tag = 8       
(p10)   frcpa f8,p12 = f6,f0                   
            nop.i 999 ;;
}

.pred.rel "mutex",p13,p14
{ .mfi
(p13)    mov        log_GR_tag = 3       
(p13)    frcpa f8,p11 = f0,f0                   
            nop.i 999
}
{ .mfb
(p14)    mov        log_GR_tag = 9       
(p14)   frcpa f8,p12 = f0,f0                   
        br.cond.sptk __libm_error_region ;; 
}
.endp log
ASM_SIZE_DIRECTIVE(log)
ASM_SIZE_DIRECTIVE(__ieee754_log)


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
        stfd [GR_Parameter_Y] = f1,16         // STORE Parameter 2 on stack
        add GR_Parameter_X = 16,sp            // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                     // Save b0
};;

.body
// (3)
{ .mib
        stfd [GR_Parameter_X] = f10                   // STORE Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y  // Parameter 3 address
        nop.b 0                             
}
{ .mib
        stfd [GR_Parameter_Y] = f8                    // STORE Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#         // Call error handling function
};;

{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;

// (4)
{ .mmi
        ldfd  f8 = [GR_Parameter_RESULT]       // Get return result off stack
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
