/* Bounded-pointer thunk definitions.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Greg McGary <greg@mcgary.org>

   This file is part of the GNU C Library.  Its master source is NOT part of
   the C library, however.  The master source lives in the GNU MP Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _bpthunks_h_
#define _bpthunks_h_

#include <libc-symbols.h>

#define BP_ALIAS(STRONG, ALIAS) weak_alias (__BP_##STRONG, __BP_##ALIAS)

#define PV(P) __ptrvalue (P)
#define SV(S) __ptrvalue (S)
#define PB(P) __ptrlow (P)
#define PE(P) __ptrhigh (P)
#define voidp void *__bounded
#define charp char *__bounded

/* GKM FIXME: Add code to check bounds.  Right now, they only strip bounds, */

#define BP_THUNK_i_iiip(NAME) __unbounded { \
  extern int NAME (int, int, int, void *); \
  int __BP_##NAME (int i0, int i1, int i2, voidp p3) \
    { return NAME (i0, i1, i2, PV (p3)); } }

#define BP_THUNK_i_iiipi(NAME) __unbounded { \
  extern int NAME (int, int, int, void *, int); \
  int __BP_##NAME (int i0, int i1, int i2, voidp p3, int i4) \
    { return NAME (i0, i1, i2, PV (p3), i4); } }

#define BP_THUNK_i_iiipp(NAME) __unbounded { \
  extern int NAME (int, int, int, void *, void *); \
  int __BP_##NAME (int i0, int i1, int i2, voidp p3, voidp p4) \
    { return NAME (i0, i1, i2, PV (p3), PV (p4)); } }

#define BP_THUNK_i_iip(NAME) __unbounded { \
  extern int NAME (int, int, void *); \
  int __BP_##NAME (int i0, int i1, voidp p2) \
    { return NAME (i0, i1, PV (p2)); } }

#define BP_THUNK_i_iipi(NAME) __unbounded { \
  extern int NAME (int, int, void *, int); \
  int __BP_##NAME (int i0, int i1, voidp p2, int i3) \
    { return NAME (i0, i1, PV (p2), i3); } }

#define BP_THUNK_i_iipp(NAME) __unbounded { \
  extern int NAME (int, int, void *, void *); \
  int __BP_##NAME (int i0, int i1, voidp p2, voidp p3) \
    { return NAME (i0, i1, PV (p2), PV (p3)); } }

#define BP_THUNK_i_ip(NAME) __unbounded { \
  extern int NAME (int, void *); \
  int __BP_##NAME (int i0, voidp p1) \
    { return NAME (i0, PV (p1)); } }

#define BP_THUNK_i_ipi(NAME) __unbounded { \
  extern int NAME (int, void *, int); \
  int __BP_##NAME (int i0, voidp p1, int i2) \
    { return NAME (i0, PV (p1), i2); } }

#define BP_THUNK_i_ipii(NAME) __unbounded { \
  extern int NAME (int, void *, int, int); \
  int __BP_##NAME (int i0, voidp p1, int i2, int i3) \
    { return NAME (i0, PV (p1), i2, i3); } }

#define BP_THUNK_i_ipiii(NAME) __unbounded { \
  extern int NAME (int, void *, int, int, int); \
  int __BP_##NAME (int i0, voidp p1, int i2, int i3, int i4) \
    { return NAME (i0, PV (p1), i2, i3, i4); } }

#define BP_THUNK_i_ipiipi(NAME) __unbounded { \
  extern int NAME (int, void *, int, int, void *, int); \
  int __BP_##NAME (int i0, voidp p1, int i2, int i3, voidp p4, int i5) \
    { return NAME (i0, PV (p1), i2, i3, PV (p4), i5); } }

#define BP_THUNK_i_ipiipp(NAME) __unbounded { \
  extern int NAME (int, void *, int, int, void *, void *); \
  int __BP_##NAME (int i0, voidp p1, int i2, int i3, voidp p4, voidp p5) \
    { return NAME (i0, PV (p1), i2, i3, PV (p4), PV (p5)); } }

#define BP_THUNK_i_ipip(NAME) __unbounded { \
  extern int NAME (int, void *, int, void *); \
  int __BP_##NAME (int i0, voidp p1, int i2, voidp p3) \
    { return NAME (i0, PV (p1), i2, PV (p3)); } }

#define BP_THUNK_i_ipp(NAME) __unbounded { \
  extern int NAME (int, void *, void *); \
  int __BP_##NAME (int i0, voidp p1, voidp p2) \
    { return NAME (i0, PV (p1), PV (p2)); } }

#define BP_THUNK_i_ippi(NAME) __unbounded { \
  extern int NAME (int, void *, void *, int); \
  int __BP_##NAME (int i0, voidp p1, voidp p2, int i3) \
    { return NAME (i0, PV (p1), PV (p2), i3); } }

#define BP_THUNK_i_ipppp(NAME) __unbounded { \
  extern int NAME (int, void *, void *, void *, void *); \
  int __BP_##NAME (int i0, voidp p1, voidp p2, voidp p3, voidp p4) \
    { return NAME (i0, PV (p1), PV (p2), PV (p3), PV (p4)); } }

#define BP_THUNK_i_isi(NAME) __unbounded { \
  extern int NAME (int, char *, int); \
  int __BP_##NAME (int i0, charp s1, int i2) \
    { return NAME (i0, SV (s1), i2); } }

#define BP_THUNK_i_isip(NAME) __unbounded { \
  extern int NAME (int, char *, int, void *); \
  int __BP_##NAME (int i0, charp s1, int i2, voidp p3) \
    { return NAME (i0, SV (s1), i2, PV (p3)); } }

#define BP_THUNK_i_p(NAME) __unbounded { \
  extern int NAME (void *); \
  int __BP_##NAME (voidp p0) \
    { return NAME (PV (p0)); } }

#define BP_THUNK_i_pi(NAME) __unbounded { \
  extern int NAME (void *, int); \
  int __BP_##NAME (voidp p0, int i1) \
    { return NAME (PV (p0), i1); } }

#define BP_THUNK_i_pii(NAME) __unbounded { \
  extern int NAME (void *, int, int); \
  int __BP_##NAME (voidp p0, int i1, int i2) \
    { return NAME (PV (p0), i1, i2); } }

#define BP_THUNK_i_piii(NAME) __unbounded { \
  extern int NAME (void *, int, int, int); \
  int __BP_##NAME (voidp p0, int i1, int i2, int i3) \
    { return NAME (PV (p0), i1, i2, i3); } }

#define BP_THUNK_i_pp(NAME) __unbounded { \
  extern int NAME (void *, void *); \
  int __BP_##NAME (voidp p0, voidp p1) \
    { return NAME (PV (p0), PV (p1)); } }

#define BP_THUNK_i_pppi(NAME) __unbounded { \
  extern int NAME (void *, void *, void *, int); \
  int __BP_##NAME (voidp p0, voidp p1, voidp p2, int i3) \
    { return NAME (PV (p0), PV (p1), PV (p2), i3); } }

#define BP_THUNK_i_s(NAME) __unbounded { \
  extern int NAME (char *); \
  int __BP_##NAME (charp s0) \
    { return NAME (SV (s0)); } }

#define BP_THUNK_i_si(NAME) __unbounded { \
  extern int NAME (char *, int); \
  int __BP_##NAME (charp s0, int i1) \
    { return NAME (SV (s0), i1); } }

#define BP_THUNK_i_sii(NAME) __unbounded { \
  extern int NAME (char *, int, int); \
  int __BP_##NAME (charp s0, int i1, int i2) \
    { return NAME (SV (s0), i1, i2); } }

#define BP_THUNK_i_sipip(NAME) __unbounded { \
  extern int NAME (char *, int, void *, int, void *); \
  int __BP_##NAME (charp s0, int i1, voidp p2, int i3, voidp p4) \
    { return NAME (SV (s0), i1, PV (p2), i3, PV (p4)); } }

#define BP_THUNK_i_sp(NAME) __unbounded { \
  extern int NAME (char *, void *); \
  int __BP_##NAME (charp s0, voidp p1) \
    { return NAME (SV (s0), PV (p1)); } }

#define BP_THUNK_i_spi(NAME) __unbounded { \
  extern int NAME (char *, void *, int); \
  int __BP_##NAME (charp s0, voidp p1, int i2) \
    { return NAME (SV (s0), PV (p1), i2); } }

#define BP_THUNK_i_spp(NAME) __unbounded { \
  extern int NAME (char *, void *, void *); \
  int __BP_##NAME (charp s0, voidp p1, voidp p2) \
    { return NAME (SV (s0), PV (p1), PV (p2)); } }

#define BP_THUNK_i_ss(NAME) __unbounded { \
  extern int NAME (char *, char *); \
  int __BP_##NAME (charp s0, charp s1) \
    { return NAME (SV (s0), s1); } }

#define BP_THUNK_i_sssip(NAME) __unbounded { \
  extern int NAME (char *, char *, char *, int, void *); \
  int __BP_##NAME (charp s0, charp s1, charp s2, int i3, voidp p4) \
    { return NAME (SV (s0), SV (s1), SV (s2), i3, PV (p4)); } }

/* sstk */
#define BP_THUNK_p_i(NAME) __unbounded { \
  extern void *NAME (int); \
  voidp __BP_##NAME (int i0) \
    { charp m; PV (m) = PB (m) = NAME (i0); \
      PE (m) = PV (m) + i0; return m; } }

/* mremap */
#define BP_THUNK_p_piii(NAME) __unbounded { \
  extern void *NAME (void *, int, int, int); \
  voidp __BP_##NAME (voidp p0, int i1, int i2, int i3) \
    { charp m; PV (m) = PB (m) = NAME (PV (p0), i1, i2, i3); \
      PE (m) = PV (m) + i2; return m; } }

/* mmap */
#define BP_THUNK_p_piiiii(NAME) __unbounded { \
  extern void *NAME (void *, int, int, int, int, int); \
  voidp __BP_##NAME (voidp p0, int i1, int i2, int i3, int i4, int i5) \
    { charp m; PV (m) = PB (m) = NAME (PV (p0), i1, i2, i3, i4, i5); \
      PE (m) = PV (m) + i1; return m; } }

#endif /* _bpthunks_h_ */
