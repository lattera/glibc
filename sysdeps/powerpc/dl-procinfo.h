/* Processor capability information handling macros.  PowerPC version.
   Copyright (C) 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _DL_PROCINFO_H
#define _DL_PROCINFO_H	1

#include <ldsodefs.h>
#include <sysdep.h>		/* This defines the PPC_FEATURE_* macros.  */

/* There are 16 bits used, but they are bits 16..31.  */
#define _DL_HWCAP_FIRST		16
#define _DL_HWCAP_COUNT		32

/* These bits influence library search.  */
#define HWCAP_IMPORTANT		(PPC_FEATURE_HAS_ALTIVEC		      \
				 | PPC_FEATURE_POWER4			      \
				 | PPC_FEATURE_POWER5			      \
				 | PPC_FEATURE_POWER5_PLUS		      \
				 | PPC_FEATURE_CELL)

/* We don't use AT_PLATFORM.  */
#define _DL_HWCAP_PLATFORM 	0
#define _dl_string_platform(str) (-1)

static inline const char *
__attribute__ ((unused))
_dl_hwcap_string (int idx)
{
  return GLRO(dl_powerpc_cap_flags)[idx - _DL_HWCAP_FIRST];
};

static inline int
__attribute__ ((unused))
_dl_string_hwcap (const char *str)
{
  for (int i = _DL_HWCAP_FIRST; i < _DL_HWCAP_COUNT; ++i)
    if (strcmp (str, _dl_hwcap_string (i)) == 0)
      return i;
  return -1;
};

#ifdef IS_IN_rtld
static inline int
__attribute__ ((unused))
_dl_procinfo (int word)
{
  _dl_printf ("AT_HWCAP:       ");

  for (int i = _DL_HWCAP_FIRST; i < _DL_HWCAP_COUNT; ++i)
    if (word & (1 << i))
      _dl_printf (" %s", _dl_hwcap_string (i));

  _dl_printf ("\n");

  return 0;
}
#endif

#endif /* dl-procinfo.h */
