/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
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

/* Like x86_64, we pass the index of the relocation and not its offset.
   In _dl_profile_fixup and _dl_call_pltexit we also use the index.
   Therefore it is wasteful to compute the offset in the trampoline
   just to reverse the operation immediately afterwards.  */
#define reloc_offset reloc_arg * sizeof (PLTREL)
#define reloc_index  reloc_arg

#include <elf/dl-runtime.c>

#include <sys/mman.h>
#include <arch/sim.h>
#include <dl-unmap-segments.h>

/* Like realpath(), but simplified: no dynamic memory use, no lstat(),
   no set_errno(), no valid "rpath" on error, etc.  This handles some
   simple cases where the simulator might not have a valid entry for
   a loaded Elf object, in particular dlopen() with a relative path.
   For this relatively rare case, one could also imagine using
   link_map.l_origin to avoid the getcwd() here, but the simpler code
   here seems like a better solution.  */
static char *
dl_realpath (const char *name, char *rpath)
{
  char *dest;
  const char *start, *end;

  if (name[0] != '/')
    {
      if (!__getcwd (rpath, PATH_MAX))
        return NULL;
      dest = __rawmemchr (rpath, '\0');
    }
  else
    {
      rpath[0] = '/';
      dest = rpath + 1;
    }

  for (start = end = name; *start; start = end)
    {
      /* Skip sequence of multiple path-separators.  */
      while (*start == '/')
	++start;

      /* Find end of path component.  */
      for (end = start; *end && *end != '/'; ++end)
	/* Nothing.  */;

      if (end - start == 0)
	break;
      else if (end - start == 1 && start[0] == '.')
	/* nothing */;
      else if (end - start == 2 && start[0] == '.' && start[1] == '.')
	{
	  /* Back up to previous component, ignore if at root already.  */
	  if (dest > rpath + 1)
	    while ((--dest)[-1] != '/');
	}
      else
	{
	  if (dest[-1] != '/')
	    *dest++ = '/';

	  if (dest + (end - start) >= rpath + PATH_MAX)
            return NULL;

	  dest = __mempcpy (dest, start, end - start);
	  *dest = '\0';
	}
    }
  if (dest > rpath + 1 && dest[-1] == '/')
    --dest;
  *dest = '\0';

  return rpath;
}

/* Support notifying the simulator about new objects.  */
void internal_function
_dl_after_load (struct link_map *l)
{
  int shift;
  char pathbuf[PATH_MAX];
  char *path;

  /* Don't bother if not in the simulator. */
  if (__insn_mfspr (SPR_SIM_CONTROL) == 0)
    return;

#define DLPUTC(c) __insn_mtspr (SPR_SIM_CONTROL,                         \
                                (SIM_CONTROL_DLOPEN                      \
                                 | ((c) << _SIM_CONTROL_OPERATOR_BITS)))

  /* Write the library address in hex.  */
  DLPUTC ('0');
  DLPUTC ('x');
  for (shift = (int) sizeof (unsigned long) * 8 - 4; shift >= 0; shift -= 4)
    DLPUTC ("0123456789abcdef"[(l->l_map_start >> shift) & 0xF]);
  DLPUTC (':');

  /* Write the library path, including the terminating '\0'.  */
  path = dl_realpath (l->l_name, pathbuf) ?: l->l_name;
  for (size_t i = 0;; i++)
    {
      DLPUTC (path[i]);
      if (path[i] == '\0')
        break;
    }
#undef DLPUTC
}

/* Support notifying the simulator about removed objects prior to munmap().  */
static void
sim_dlclose (ElfW(Addr) map_start)
{
  int shift;

  /* Don't bother if not in the simulator.  */
  if (__insn_mfspr (SPR_SIM_CONTROL) == 0)
    return;

#define DLPUTC(c) __insn_mtspr (SPR_SIM_CONTROL,                         \
                                (SIM_CONTROL_DLCLOSE                     \
                                 | ((c) << _SIM_CONTROL_OPERATOR_BITS)))

  /* Write the library address in hex.  */
  DLPUTC ('0');
  DLPUTC ('x');
  for (shift = (int) sizeof (unsigned long) * 8 - 4; shift >= 0; shift -= 4)
    DLPUTC ("0123456789abcdef"[(map_start >> shift) & 0xF]);
  DLPUTC ('\0');

#undef DLPUTC
}

void internal_function
_dl_unmap (struct link_map *map)
{
  sim_dlclose (map->l_map_start);
  _dl_unmap_segments (map);
}
