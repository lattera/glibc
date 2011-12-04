/* Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifdef __tilegx__
#define TLS_GD_REL "hw0_last_tls_gd"
#define TLS_IE_REL "hw0_last_tls_ie"
#define LD_TLS "ld_tls"
#else
#define TLS_GD_REL "tls_gd_lo16"
#define TLS_IE_REL "tls_ie_lo16"
#define LD_TLS "lw_tls"
#endif

#define TLS_GD(x)                                               \
  ({                                                            \
    int *__retval;                                              \
    extern char _GLOBAL_OFFSET_TABLE_[];                        \
                                                                \
    asm ("addli r0, %1, " TLS_GD_REL "(" #x ")\n\t"             \
         "jal tls_gd_call(" #x ")\n\t"                          \
         "addi %0, r0, tls_gd_add(" #x ")" :                    \
         "=r" (__retval) : "r" (_GLOBAL_OFFSET_TABLE_) :        \
         "r25", "r26", "r27", "r28", "r29");                    \
    __retval; })

/* No special support for LD mode. */
#define TLS_LD TLS_GD

#define TLS_IE(x)                                               \
  ({                                                            \
    int *__retval;                                              \
    extern char _GLOBAL_OFFSET_TABLE_[];                        \
                                                                \
    asm ("addli %0, %1, " TLS_IE_REL "(" #x ")\n\t"             \
         LD_TLS " %0, %0, tls_ie_load(" #x ")\n\t"               \
         "add %0, %0, tp" :                                     \
         "=r" (__retval) : "r" (_GLOBAL_OFFSET_TABLE_));        \
    __retval; })

/* No special support for LE mode. */
#define TLS_LE TLS_IE
