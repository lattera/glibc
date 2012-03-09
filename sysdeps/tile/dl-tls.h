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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */


/* Type used for the representation of TLS information in the GOT.  */
typedef struct
{
  unsigned long int ti_module;
  unsigned long int ti_offset;
} tls_index;

/* Fast-path function to get a TLS pointer.  */
extern void *__tls_get_addr (tls_index *ti);

/* The thread pointer points to the first static TLS block.  */
#define TLS_TP_OFFSET		0

/* Dynamic thread vector pointers at the start of each TLS block.  */
#define TLS_DTV_OFFSET		0

/* Compute the value for a GOTTPREL reloc.  */
#define TLS_TPREL_VALUE(sym_map, sym) \
  ((sym_map)->l_tls_offset + (sym)->st_value - TLS_TP_OFFSET)

/* Compute the value for a DTPREL reloc.  */
#define TLS_DTPREL_VALUE(sym) \
  ((sym)->st_value - TLS_DTV_OFFSET)

/* Value used for dtv entries for which the allocation is delayed.  */
#define TLS_DTV_UNALLOCATED    ((void *) -1l)
