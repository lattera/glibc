/* Unmap a shared object's segments.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#ifndef _DL_UNMAP_SEGMENTS_H
#define _DL_UNMAP_SEGMENTS_H    1

#include <link.h>
#include <sys/mman.h>

/* There is always a big gap between the executable segment and the data
   segments.  Other code segments and data pages lie in there.  So we must
   unmap each segment individually (except for a codeless module).  */

static void __always_inline
_dl_unmap_segments (struct link_map *l)
{
  if (l->l_contiguous)
    /* Simple case.  */
    __munmap ((void *) l->l_map_start, l->l_map_end - l->l_map_start);
  else
    {
      /* Normally l_phdr points into the RODATA segment, which we will
         unmap in one iteration of the loop.  So we cannot use it directly
         throughout.  */

      struct { ElfW(Addr) start, end; } segments[l->l_phnum], *seg = segments;

      for (const ElfW(Phdr) *ph = l->l_phdr; ph < &l->l_phdr[l->l_phnum]; ++ph)
        if (ph->p_type == PT_LOAD)
          {
            seg->start = (l->l_addr + ph->p_vaddr) & -GLRO(dl_pagesize);
            seg->end = (l->l_addr + ph->p_vaddr + ph->p_memsz
                        + GLRO(dl_pagesize) - 1) & -GLRO(dl_pagesize);
            if (seg > segments && seg[-1].end == seg->start)
              /* Coalesce two adjacent segments into one munmap call.  */
              seg[-1].end = seg->end;
            else
              ++seg;
          }

      do
        {
          --seg;
          __munmap ((void *) seg->start, seg->end - seg->start);
        }
      while (seg > segments);
    }
}

#endif  /* dl-unmap-segments.h */
