/* Copyright (C) 2011-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef __tilegx__
#define TLS_GD_OFFSET(x)                        \
  "moveli r0, hw1_last_tls_gd(" #x ")\n\t"      \
  "shl16insli r0, r0, hw0_tls_gd(" #x ")\n\t"   \
  "addi r0, %1, tls_add(" #x ")\n\t"
#else
#define TLS_GD_OFFSET(x)                        \
  "auli r0, %1, tls_gd_ha16(" #x ")\n\t"        \
  "addli r0, r0, tls_gd_lo16(" #x ")\n\t"
#endif

#define TLS_GD(x)                                               \
  ({                                                            \
    int *__retval;                                              \
    extern char _GLOBAL_OFFSET_TABLE_[];                        \
                                                                \
    asm (TLS_GD_OFFSET(x)                                       \
         "jal tls_gd_call(" #x ")\n\t"                          \
         "addi %0, r0, tls_gd_add(" #x ")" :                    \
         "=&r" (__retval) : "r" (_GLOBAL_OFFSET_TABLE_) :       \
         "r0", "r25", "r26", "r27", "r28", "r29");              \
    __retval; })

/* No special support for LD mode. */
#define TLS_LD TLS_GD

#ifdef __tilegx__
#define TLS_IE_OFFSET(x)                        \
  "moveli %0, hw1_last_tls_ie(" #x ")\n\t"      \
  "shl16insli %0, %0, hw0_tls_ie(" #x ")\n\t"   \
  "addi %0, %1, tls_add(" #x ")\n\t"
#define LD_TLS "ld_tls"
#else
#define TLS_IE_OFFSET(x)                        \
  "auli %0, %1, tls_ie_ha16(" #x ")\n\t"        \
  "addli %0, %0, tls_ie_lo16(" #x ")\n\t"
#define LD_TLS "lw_tls"
#endif

#define TLS_IE(x)                                               \
  ({                                                            \
    int *__retval;                                              \
    extern char _GLOBAL_OFFSET_TABLE_[];                        \
                                                                \
    asm (TLS_IE_OFFSET(x)                                       \
         LD_TLS " %0, %0, tls_ie_load(" #x ")\n\t"              \
         "add %0, %0, tp" :                                     \
         "=&r" (__retval) : "r" (_GLOBAL_OFFSET_TABLE_));       \
    __retval; })

#ifdef __tilegx__
#define _TLS_LE(x)                              \
  "moveli %0, hw1_last_tls_le(" #x ")\n\t"      \
  "shl16insli %0, %0, hw0_tls_le(" #x ")\n\t"   \
  "add %0, %0, tp"
#else
#define _TLS_LE(x)                              \
  "auli %0, tp, tls_le_ha16(" #x ")\n\t"        \
  "addli %0, %0, tls_le_lo16(" #x ")\n\t"
#endif

#define TLS_LE(x)                               \
  ({                                            \
    int *__retval;                              \
    asm (_TLS_LE(x) : "=r" (__retval));         \
    __retval; })
