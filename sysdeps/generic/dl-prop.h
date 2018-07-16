/* Support for GNU properties.  Generic version.
   Copyright (C) 2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _DL_PROP_H
#define _DL_PROP_H

/* The following functions are used by the dynamic loader and the
   dlopen machinery to process PT_NOTE entries in the binary or
   shared object.  The notes can be used to change the behaviour of
   the loader, and as such offer a flexible mechanism for hooking in
   various checks related to ABI tags or implementing "flag day" ABI
   transitions.  */

static inline void __attribute__ ((always_inline))
_rtld_main_check (struct link_map *m, const char *program)
{
}

static inline void __attribute__ ((always_inline))
_dl_open_check (struct link_map *m)
{
}

#ifdef FILEBUF_SIZE
static inline int __attribute__ ((always_inline))
_dl_process_pt_note (struct link_map *l, const ElfW(Phdr) *ph,
		     int fd, struct filebuf *fbp)
{
  return 0;
}
#endif

static inline int __attribute__ ((always_inline))
_rtld_process_pt_note (struct link_map *l, const ElfW(Phdr) *ph)
{
  return 0;
}

#endif /* _DL_PROP_H */
